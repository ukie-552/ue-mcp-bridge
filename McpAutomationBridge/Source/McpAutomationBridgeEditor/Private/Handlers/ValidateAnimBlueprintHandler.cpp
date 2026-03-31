#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"

class FMcpValidateAnimBlueprintHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("validate_anim_blueprint"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bCheckConnections = true;
        Params->TryGetBoolField(TEXT("check_connections"), bCheckConnections);

        bool bCheckVariables = true;
        Params->TryGetBoolField(TEXT("check_variables"), bCheckVariables);

        bool bCheckStateMachines = true;
        Params->TryGetBoolField(TEXT("check_state_machines"), bCheckStateMachines);

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> Issues;
        TArray<TSharedPtr<FJsonValue>> Warnings;

        if (bCheckVariables)
        {
            ValidateVariables(AnimBP, Issues, Warnings);
        }

        if (bCheckConnections)
        {
            ValidateConnections(AnimBP, Issues, Warnings);
        }

        if (bCheckStateMachines)
        {
            ValidateStateMachines(AnimBP, Issues, Warnings);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("is_valid"), Issues.Num() == 0);
        Result->SetNumberField(TEXT("issue_count"), Issues.Num());
        Result->SetNumberField(TEXT("warning_count"), Warnings.Num());
        Result->SetArrayField(TEXT("issues"), Issues);
        Result->SetArrayField(TEXT("warnings"), Warnings);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetStringField(TEXT("anim_blueprint_name"), AnimBP->GetName());

        TSharedPtr<FJsonObject> Stats = MakeShareable(new FJsonObject);
        Stats->SetNumberField(TEXT("variable_count"), AnimBP->NewVariables.Num());
        Stats->SetNumberField(TEXT("function_count"), AnimBP->FunctionGraphs.Num());
        Stats->SetNumberField(TEXT("macro_count"), AnimBP->MacroGraphs.Num());
        Stats->SetNumberField(TEXT("animation_layer_count"), AnimBP->AnimationLayerGraphs.Num());
        Stats->SetNumberField(TEXT("ubergraph_count"), AnimBP->UbergraphPages.Num());
        Result->SetObjectField(TEXT("statistics"), Stats);

        return FMcpCommandResult::Success(Result);
    }

private:
    void ValidateVariables(UAnimBlueprint* AnimBP, TArray<TSharedPtr<FJsonValue>>& OutIssues, TArray<TSharedPtr<FJsonValue>>& OutWarnings)
    {
        TSet<FName> VariableNames;

        for (int32 i = 0; i < AnimBP->NewVariables.Num(); ++i)
        {
            const FBPVariableDescription& Var = AnimBP->NewVariables[i];

            if (Var.VarName.IsNone() || Var.VarName.ToString().IsEmpty())
            {
                TSharedPtr<FJsonObject> Issue = MakeShareable(new FJsonObject);
                Issue->SetStringField(TEXT("type"), TEXT("empty_variable_name"));
                Issue->SetStringField(TEXT("severity"), TEXT("error"));
                Issue->SetStringField(TEXT("message"), TEXT("Variable has empty name"));
                Issue->SetNumberField(TEXT("variable_index"), i);
                OutIssues.Add(MakeShareable(new FJsonValueObject(Issue)));
            }
            else if (VariableNames.Contains(Var.VarName))
            {
                TSharedPtr<FJsonObject> Issue = MakeShareable(new FJsonObject);
                Issue->SetStringField(TEXT("type"), TEXT("duplicate_variable_name"));
                Issue->SetStringField(TEXT("severity"), TEXT("error"));
                Issue->SetStringField(TEXT("message"), FString::Printf(TEXT("Duplicate variable name: %s"), *Var.VarName.ToString()));
                Issue->SetStringField(TEXT("variable_name"), Var.VarName.ToString());
                OutIssues.Add(MakeShareable(new FJsonValueObject(Issue)));
            }
            else
            {
                VariableNames.Add(Var.VarName);
            }

            if (Var.VarType.PinCategory.IsNone())
            {
                TSharedPtr<FJsonObject> Warning = MakeShareable(new FJsonObject);
                Warning->SetStringField(TEXT("type"), TEXT("invalid_variable_type"));
                Warning->SetStringField(TEXT("severity"), TEXT("warning"));
                Warning->SetStringField(TEXT("message"), FString::Printf(TEXT("Variable '%s' has invalid type"), *Var.VarName.ToString()));
                Warning->SetStringField(TEXT("variable_name"), Var.VarName.ToString());
                OutWarnings.Add(MakeShareable(new FJsonValueObject(Warning)));
            }
        }
    }

    void ValidateConnections(UAnimBlueprint* AnimBP, TArray<TSharedPtr<FJsonValue>>& OutIssues, TArray<TSharedPtr<FJsonValue>>& OutWarnings)
    {
        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(AnimBP->UbergraphPages);
        AllGraphs.Append(AnimBP->FunctionGraphs);
        AllGraphs.Append(AnimBP->MacroGraphs);
        AllGraphs.Append(AnimBP->AnimationLayerGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (!Node) continue;

                for (UEdGraphPin* Pin : Node->Pins)
                {
                    if (!Pin) continue;

                    if (Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() == 0)
                    {
                        if (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec &&
                            !Pin->PinType.IsArray() &&
                            !Pin->PinType.bIsReference)
                        {
                            TSharedPtr<FJsonObject> Warning = MakeShareable(new FJsonObject);
                            Warning->SetStringField(TEXT("type"), TEXT("unconnected_input_pin"));
                            Warning->SetStringField(TEXT("severity"), TEXT("warning"));
                            Warning->SetStringField(TEXT("message"), FString::Printf(TEXT("Input pin '%s' on node '%s' is not connected"),
                                *Pin->PinName, *Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString())));
                            Warning->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
                            Warning->SetStringField(TEXT("pin_name"), Pin->PinName);
                            Warning->SetStringField(TEXT("graph_name"), Graph->GetName());
                            OutWarnings.Add(MakeShareable(new FJsonValueObject(Warning)));
                        }
                    }
                }
            }
        }
    }

    void ValidateStateMachines(UAnimBlueprint* AnimBP, TArray<TSharedPtr<FJsonValue>>& OutIssues, TArray<TSharedPtr<FJsonValue>>& OutWarnings)
    {
        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(AnimBP->UbergraphPages);
        AllGraphs.Append(AnimBP->FunctionGraphs);
        AllGraphs.Append(AnimBP->MacroGraphs);
        AllGraphs.Append(AnimBP->AnimationLayerGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (!Node) continue;

                FString NodeClassName = Node->GetClass()->GetName();

                if (NodeClassName.Contains(TEXT("StateMachine")))
                {
                    bool bHasOutput = false;
                    bool bHasAnyState = false;

                    for (UEdGraphPin* Pin : Node->Pins)
                    {
                        if (Pin && Pin->PinName == TEXT("Result") && Pin->LinkedTo.Num() > 0)
                        {
                            bHasOutput = true;
                        }
                    }

                    if (!bHasOutput)
                    {
                        TSharedPtr<FJsonObject> Warning = MakeShareable(new FJsonObject);
                        Warning->SetStringField(TEXT("type"), TEXT("state_machine_no_output"));
                        Warning->SetStringField(TEXT("severity"), TEXT("warning"));
                        Warning->SetStringField(TEXT("message"), FString::Printf(TEXT("State Machine '%s' has no output connection"),
                            *Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString())));
                        Warning->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
                        Warning->SetStringField(TEXT("graph_name"), Graph->GetName());
                        OutWarnings.Add(MakeShareable(new FJsonValueObject(Warning)));
                    }
                }
            }
        }
    }
};

REGISTER_MCP_COMMAND(FMcpValidateAnimBlueprintHandler)