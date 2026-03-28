#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_StateMachine.h"
#include "AnimGraphNode_StateResult.h"
#include "AnimGraphNode_TransitionResult.h"
#include "AnimationStateMachineSchema.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"

class FMcpAddStateTransitionHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_state_transition"); }

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

        FString FromState;
        if (!Params->TryGetStringField(TEXT("from_state"), FromState))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'from_state' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ToState;
        if (!Params->TryGetStringField(TEXT("to_state"), ToState))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'to_state' parameter"), TEXT("MISSING_PARAM"));
        }

        FString TransitionRule;
        Params->TryGetStringField(TEXT("transition_rule"), TransitionRule);

        bool bAutomaticTransition = false;
        Params->TryGetBoolField(TEXT("automatic"), bAutomaticTransition);

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

        UEdGraphNode* FromNode = FindStateNode(StateMachineGraph, FromState);
        UEdGraphNode* ToNode = FindStateNode(StateMachineGraph, ToState);

        if (!FromNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Source state not found: %s"), *FromState),
                TEXT("FROM_STATE_NOT_FOUND")
            );
        }

        if (!ToNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Target state not found: %s"), *ToState),
                TEXT("TO_STATE_NOT_FOUND")
            );
        }

        bool bSuccess = CreateTransition(AnimBP, StateMachineGraph, FromNode, ToNode, TransitionRule, bAutomaticTransition);

        if (!bSuccess)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to create state transition"), TEXT("TRANSITION_FAILED"));
        }

        StateMachineGraph->NotifyGraphChanged();
        FBlueprintEditorUtils::MarkBlueprintAsModified(AnimBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("from_state"), FromState);
        Result->SetStringField(TEXT("to_state"), ToState);
        Result->SetStringField(TEXT("state_machine_name"), StateMachineName);

        return FMcpCommandResult::Success(Result);
    }

private:
    UEdGraph* FindStateMachineGraph(UAnimBlueprint* AnimBP, const FString& StateMachineName)
    {
        if (!AnimBP)
        {
            return nullptr;
        }

        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(AnimBP->UbergraphPages);
        AllGraphs.Append(AnimBP->FunctionGraphs);
        AllGraphs.Append(AnimBP->MacroGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph)
            {
                continue;
            }

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (UAnimGraphNode_StateMachine* SMNode = Cast<UAnimGraphNode_StateMachine>(Node))
                {
                    if (SMNode->GetStateMachineName() == StateMachineName)
                    {
                        return Cast<UEdGraph>(SMNode->EditorStateMachineGraph);
                    }
                }
            }
        }

        return nullptr;
    }

    UEdGraphNode* FindStateNode(UEdGraph* Graph, const FString& StateName)
    {
        if (!Graph)
        {
            return nullptr;
        }

        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (Node && Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString() == StateName)
            {
                return Node;
            }
        }

        return nullptr;
    }

    bool CreateTransition(UAnimBlueprint* AnimBP, UEdGraph* StateMachineGraph, UEdGraphNode* FromNode, UEdGraphNode* ToNode, const FString& TransitionRule, bool bAutomatic)
    {
        if (!StateMachineGraph || !FromNode || !ToNode)
        {
            return false;
        }

        const UEdGraphSchema* Schema = StateMachineGraph->GetSchema();
        if (!Schema)
        {
            return false;
        }

        UEdGraphPin* FromOutputPin = nullptr;
        for (UEdGraphPin* Pin : FromNode->Pins)
        {
            if (Pin && Pin->Direction == EGPD_Output)
            {
                FromOutputPin = Pin;
                break;
            }
        }

        UEdGraphPin* ToInputPin = nullptr;
        for (UEdGraphPin* Pin : ToNode->Pins)
        {
            if (Pin && Pin->Direction == EGPD_Input)
            {
                ToInputPin = Pin;
                break;
            }
        }

        if (FromOutputPin && ToInputPin)
        {
            Schema->TryCreateConnection(FromOutputPin, ToInputPin);
            return true;
        }

        return false;
    }
};

REGISTER_MCP_COMMAND(FMcpAddStateTransitionHandler)
