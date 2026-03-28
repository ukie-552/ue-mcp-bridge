#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_StateMachine.h"
#include "AnimGraphNode_StateResult.h"
#include "AnimGraphNode_Base.h"
#include "AnimationStateMachineSchema.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"

class FMcpAddAnimStateHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_anim_state"); }

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

        int32 PosX = 0, PosY = 0;
        const TSharedPtr<FJsonObject>* PositionObj;
        if (Params->TryGetObjectField(TEXT("position"), PositionObj))
        {
            (*PositionObj)->TryGetNumberField(TEXT("x"), PosX);
            (*PositionObj)->TryGetNumberField(TEXT("y"), PosY);
        }

        bool bIsEntryState = false;
        Params->TryGetBoolField(TEXT("is_entry_state"), bIsEntryState);

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UEdGraph* StateMachineGraph = FindStateMachineGraph(AnimBP, StateMachineName);
        if (!StateMachineGraph)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("State machine not found: %s"), *StateMachineName),
                TEXT("STATE_MACHINE_NOT_FOUND")
            );
        }

        UEdGraphNode* NewStateNode = CreateStateNode(StateMachineGraph, StateName, PosX, PosY, bIsEntryState);
        if (!NewStateNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create state: %s"), *StateName),
                TEXT("STATE_CREATION_FAILED")
            );
        }

        StateMachineGraph->NotifyGraphChanged();
        FBlueprintEditorUtils::MarkBlueprintAsModified(AnimBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("state_name"), StateName);
        Result->SetStringField(TEXT("state_machine_name"), StateMachineName);
        Result->SetStringField(TEXT("node_id"), NewStateNode->NodeGuid.ToString());
        Result->SetNumberField(TEXT("pos_x"), NewStateNode->NodePosX);
        Result->SetNumberField(TEXT("pos_y"), NewStateNode->NodePosY);
        Result->SetBoolField(TEXT("is_entry_state"), bIsEntryState);

        return FMcpCommandResult::Success(Result);
    }

private:
    UEdGraph* FindStateMachineGraph(UAnimBlueprint* AnimBP, const FString& StateMachineName)
    {
        if (!AnimBP)
        {
            return nullptr;
        }

        for (UEdGraph* Graph : AnimBP->UbergraphPages)
        {
            if (Graph && Graph->GetName() == StateMachineName)
            {
                return Graph;
            }
        }

        for (UEdGraphNode* Node : AnimBP->GetAnimGraph()->Nodes)
        {
            if (UAnimGraphNode_StateMachine* SMNode = Cast<UAnimGraphNode_StateMachine>(Node))
            {
                if (SMNode->StateMachineName.ToString() == StateMachineName)
                {
                    return SMNode->GetBoundGraph();
                }
            }
        }

        return nullptr;
    }

    UEdGraphNode* CreateStateNode(UEdGraph* Graph, const FString& StateName, int32 PosX, int32 PosY, bool bIsEntryState)
    {
        if (!Graph)
        {
            return nullptr;
        }

        UClass* NodeClass = FindObject<UClass>(nullptr, TEXT("/Script/AnimGraph.AnimStateNode"));
        if (!NodeClass)
        {
            NodeClass = UEdGraphNode::StaticClass();
        }

        UEdGraphNode* NewNode = NewObject<UEdGraphNode>(Graph, NodeClass);
        NewNode->SetFlags(RF_Transactional);
        Graph->AddNode(NewNode, false, false);

        NewNode->NodePosX = PosX;
        NewNode->NodePosY = PosY;
        NewNode->CreateNewGuid();
        NewNode->PostPlacedNewNode();
        NewNode->AllocateDefaultPins();

        return NewNode;
    }
};

REGISTER_MCP_COMMAND(FMcpAddAnimStateHandler)
