#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "AnimGraphNode_StateMachine.h"
#include "AnimGraphNode_Base.h"
#include "AnimGraphNode_SequencePlayer.h"
#include "AnimGraphNode_BlendSpacePlayer.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"

class FMcpSetStateAnimationHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_state_animation"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString StateMachineName;
        if (!Params->TryGetStringField(TEXT("state_machine_name"), StateMachineName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'state_machine_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString StateName;
        if (!Params->TryGetStringField(TEXT("state_name"), StateName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'state_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString AnimationPath;
        if (!Params->TryGetStringField(TEXT("animation_path"), AnimationPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'animation_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NodeType;
        Params->TryGetStringField(TEXT("node_type"), NodeType);

        int32 PosX = 0, PosY = 0;
        const TSharedPtr<FJsonObject>* PositionObj;
        if (Params->TryGetObjectField(TEXT("position"), PositionObj))
        {
            (*PositionObj)->TryGetNumberField(TEXT("x"), PosX);
            (*PositionObj)->TryGetNumberField(TEXT("y"), PosY);
        }

        double PlayRate = 1.0;
        Params->TryGetNumberField(TEXT("play_rate"), PlayRate);

        bool bLoop = true;
        Params->TryGetBoolField(TEXT("loop"), bLoop);

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UEdGraph* StateGraph = FindStateGraph(AnimBP, StateMachineName, StateName);
        if (!StateGraph)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("State graph not found: %s"), *StateName),
                TEXT("STATE_NOT_FOUND")
            );
        }

        UAnimationAsset* AnimAsset = LoadObject<UAnimationAsset>(nullptr, *AnimationPath);
        if (!AnimAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation asset not found: %s"), *AnimationPath),
                TEXT("ANIMATION_NOT_FOUND")
            );
        }

        UEdGraphNode* AnimNode = CreateAnimationNode(StateGraph, AnimAsset, NodeType, PosX, PosY, PlayRate, bLoop);
        if (!AnimNode)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to create animation node"), TEXT("NODE_CREATION_FAILED"));
        }

        StateGraph->NotifyGraphChanged();
        FBlueprintEditorUtils::MarkBlueprintAsModified(AnimBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), true);
        Result->SetStringField(TEXT("state_name"), StateName);
        Result->SetStringField(TEXT("animation_path"), AnimationPath);
        Result->SetStringField(TEXT("animation_name"), AnimAsset->GetName());
        Result->SetStringField(TEXT("node_id"), AnimNode->NodeGuid.ToString());

        return FMcpCommandResult::Success(Result);
    }

private:
    UEdGraph* FindStateGraph(UAnimBlueprint* AnimBP, const FString& StateMachineName, const FString& StateName)
    {
        if (!AnimBP)
        {
            return nullptr;
        }

        UEdGraph* AnimGraph = AnimBP->GetAnimGraph();
        if (!AnimGraph)
        {
            return nullptr;
        }

        for (UEdGraphNode* Node : AnimGraph->Nodes)
        {
            if (UAnimGraphNode_StateMachine* SMNode = Cast<UAnimGraphNode_StateMachine>(Node))
            {
                if (SMNode->StateMachineName.ToString() == StateMachineName)
                {
                    UEdGraph* SMGraph = SMNode->GetBoundGraph();
                    if (SMGraph)
                    {
                        for (UEdGraphNode* StateNode : SMGraph->Nodes)
                        {
                            if (StateNode && StateNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString() == StateName)
                            {
                                for (UEdGraph* InnerGraph : AnimBP->UbergraphPages)
                                {
                                    if (InnerGraph && InnerGraph->GetName().Contains(StateName))
                                    {
                                        return InnerGraph;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return nullptr;
    }

    UEdGraphNode* CreateAnimationNode(UEdGraph* Graph, UAnimationAsset* AnimAsset, const FString& NodeType, int32 PosX, int32 PosY, double PlayRate, bool bLoop)
    {
        if (!Graph || !AnimAsset)
        {
            return nullptr;
        }

        UEdGraphNode* NewNode = nullptr;

        if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(AnimAsset))
        {
            UAnimGraphNode_SequencePlayer* SeqNode = NewObject<UAnimGraphNode_SequencePlayer>(Graph);
            SeqNode->SetFlags(RF_Transactional);
            Graph->AddNode(SeqNode, false, false);

            SeqNode->Node = UAnimGraphNode_SequencePlayer::FNode();
            SeqNode->Node.Sequence = AnimSequence;
            SeqNode->Node.PlayRate = static_cast<float>(PlayRate);
            SeqNode->Node.bLoop = bLoop;

            NewNode = SeqNode;
        }
        else if (UBlendSpace* BlendSpace = Cast<UBlendSpace>(AnimAsset))
        {
            UAnimGraphNode_BlendSpacePlayer* BSNode = NewObject<UAnimGraphNode_BlendSpacePlayer>(Graph);
            BSNode->SetFlags(RF_Transactional);
            Graph->AddNode(BSNode, false, false);

            BSNode->Node.BlendSpace = BlendSpace;
            BSNode->Node.PlayRate = static_cast<float>(PlayRate);
            BSNode->Node.bLoop = bLoop;

            NewNode = BSNode;
        }

        if (NewNode)
        {
            NewNode->NodePosX = PosX;
            NewNode->NodePosY = PosY;
            NewNode->CreateNewGuid();
            NewNode->PostPlacedNewNode();
            NewNode->AllocateDefaultPins();
        }

        return NewNode;
    }
};

REGISTER_MCP_COMMAND(FMcpSetStateAnimationHandler)
