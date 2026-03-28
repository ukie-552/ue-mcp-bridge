#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_StateMachine.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"

class FMcpCreateStateMachineHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_state_machine"); }

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

        FString GraphName;
        Params->TryGetStringField(TEXT("graph_name"), GraphName);

        int32 PosX = 0, PosY = 0;
        const TSharedPtr<FJsonObject>* PositionObj;
        if (Params->TryGetObjectField(TEXT("position"), PositionObj))
        {
            (*PositionObj)->TryGetNumberField(TEXT("x"), PosX);
            (*PositionObj)->TryGetNumberField(TEXT("y"), PosY);
        }

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UEdGraph* TargetGraph = nullptr;

        if (!GraphName.IsEmpty())
        {
            for (UEdGraph* Graph : AnimBP->UbergraphPages)
            {
                if (Graph && Graph->GetName() == GraphName)
                {
                    TargetGraph = Graph;
                    break;
                }
            }
        }

        if (!TargetGraph)
        {
            TArray<UEdGraph*> AllGraphs;
            AllGraphs.Append(AnimBP->UbergraphPages);
            AllGraphs.Append(AnimBP->FunctionGraphs);
            
            for (UEdGraph* Graph : AllGraphs)
            {
                if (Graph)
                {
                    TargetGraph = Graph;
                    break;
                }
            }
        }

        if (!TargetGraph)
        {
            return FMcpCommandResult::Failure(TEXT("No anim graph found"), TEXT("NO_ANIM_GRAPH"));
        }

        UClass* StateMachineNodeClass = LoadClass<UEdGraphNode>(nullptr, TEXT("/Script/AnimGraph.AnimGraphNode_StateMachine"));
        if (!StateMachineNodeClass)
        {
            return FMcpCommandResult::Failure(TEXT("State machine node class not found"), TEXT("CLASS_NOT_FOUND"));
        }

        UEdGraphNode* StateMachineNode = NewObject<UEdGraphNode>(TargetGraph, StateMachineNodeClass);
        if (!StateMachineNode)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to create state machine node"), TEXT("NODE_CREATION_FAILED"));
        }

        StateMachineNode->SetFlags(RF_Transactional);
        TargetGraph->AddNode(StateMachineNode, false, false);

        StateMachineNode->NodePosX = PosX;
        StateMachineNode->NodePosY = PosY;
        StateMachineNode->CreateNewGuid();
        StateMachineNode->PostPlacedNewNode();
        StateMachineNode->AllocateDefaultPins();

        TargetGraph->NotifyGraphChanged();
        FBlueprintEditorUtils::MarkBlueprintAsModified(AnimBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("state_machine_name"), StateMachineName);
        Result->SetStringField(TEXT("node_id"), StateMachineNode->NodeGuid.ToString());
        Result->SetNumberField(TEXT("pos_x"), StateMachineNode->NodePosX);
        Result->SetNumberField(TEXT("pos_y"), StateMachineNode->NodePosY);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateStateMachineHandler)
