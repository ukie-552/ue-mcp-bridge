#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_StateMachine.h"
#include "AnimGraphNode_StateResult.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"

class FMcpGetStateMachineInfoHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_state_machine_info"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString StateMachineName;
        Params->TryGetStringField(TEXT("state_machine_name"), StateMachineName);

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> StateMachinesArray;

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
                    FString SMName = SMNode->GetStateMachineName();

                    if (!StateMachineName.IsEmpty() && SMName != StateMachineName)
                    {
                        continue;
                    }

                    TSharedPtr<FJsonObject> SMInfo = MakeShareable(new FJsonObject);
                    SMInfo->SetStringField(TEXT("name"), SMName);
                    SMInfo->SetStringField(TEXT("node_id"), SMNode->NodeGuid.ToString());
                    SMInfo->SetNumberField(TEXT("pos_x"), SMNode->NodePosX);
                    SMInfo->SetNumberField(TEXT("pos_y"), SMNode->NodePosY);

                    UEdGraph* SMGraph = Cast<UEdGraph>(SMNode->EditorStateMachineGraph);
                    if (SMGraph)
                    {
                        TArray<TSharedPtr<FJsonValue>> StatesArray;
                        TArray<TSharedPtr<FJsonValue>> TransitionsArray;

                        for (UEdGraphNode* StateNode : SMGraph->Nodes)
                        {
                            if (StateNode)
                            {
                                TSharedPtr<FJsonObject> StateInfo = MakeShareable(new FJsonObject);
                                StateInfo->SetStringField(TEXT("name"), StateNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
                                StateInfo->SetStringField(TEXT("node_id"), StateNode->NodeGuid.ToString());
                                StateInfo->SetNumberField(TEXT("pos_x"), StateNode->NodePosX);
                                StateInfo->SetNumberField(TEXT("pos_y"), StateNode->NodePosY);

                                StatesArray.Add(MakeShareable(new FJsonValueObject(StateInfo)));

                                for (UEdGraphPin* Pin : StateNode->Pins)
                                {
                                    if (Pin && Pin->Direction == EGPD_Output)
                                    {
                                        for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
                                        {
                                            if (LinkedPin && LinkedPin->GetOwningNode())
                                            {
                                                TSharedPtr<FJsonObject> TransInfo = MakeShareable(new FJsonObject);
                                                TransInfo->SetStringField(TEXT("from_state"), StateNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
                                                TransInfo->SetStringField(TEXT("to_state"), LinkedPin->GetOwningNode()->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
                                                TransitionsArray.Add(MakeShareable(new FJsonValueObject(TransInfo)));
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        SMInfo->SetArrayField(TEXT("states"), StatesArray);
                        SMInfo->SetArrayField(TEXT("transitions"), TransitionsArray);
                        SMInfo->SetNumberField(TEXT("state_count"), StatesArray.Num());
                    }

                    StateMachinesArray.Add(MakeShareable(new FJsonValueObject(SMInfo)));
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetArrayField(TEXT("state_machines"), StateMachinesArray);
        Result->SetNumberField(TEXT("state_machine_count"), StateMachinesArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetStateMachineInfoHandler)
