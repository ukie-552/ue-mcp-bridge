#include "CoreMinimal.h"
#include "McpCommand.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "K2Node.h"

class FMcpValidateBlueprintHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("validate_blueprint"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bCheckConnections = true;
        Params->TryGetBoolField(TEXT("check_connections"), bCheckConnections);

        bool bCheckVariables = true;
        Params->TryGetBoolField(TEXT("check_variables"), bCheckVariables);

        bool bCheckFunctions = true;
        Params->TryGetBoolField(TEXT("check_functions"), bCheckFunctions);

        UBlueprint* Blueprint = LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> Issues;
        TArray<TSharedPtr<FJsonValue>> Warnings;

        if (bCheckVariables)
        {
            ValidateVariables(Blueprint, Issues, Warnings);
        }

        if (bCheckFunctions)
        {
            ValidateFunctions(Blueprint, Issues, Warnings);
        }

        if (bCheckConnections)
        {
            ValidateConnections(Blueprint, Issues, Warnings);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("is_valid"), Issues.Num() == 0);
        Result->SetNumberField(TEXT("issue_count"), Issues.Num());
        Result->SetNumberField(TEXT("warning_count"), Warnings.Num());
        Result->SetArrayField(TEXT("issues"), Issues);
        Result->SetArrayField(TEXT("warnings"), Warnings);

        TSharedPtr<FJsonObject> Stats = MakeShareable(new FJsonObject);
        Stats->SetNumberField(TEXT("variable_count"), Blueprint->NewVariables.Num());
        Stats->SetNumberField(TEXT("function_count"), Blueprint->FunctionGraphs.Num());
        Stats->SetNumberField(TEXT("macro_count"), Blueprint->MacroGraphs.Num());
        Stats->SetNumberField(TEXT("event_graph_count"), Blueprint->UbergraphPages.Num());
        Result->SetObjectField(TEXT("statistics"), Stats);

        return FMcpCommandResult::Success(Result);
    }

private:
    UBlueprint* LoadBlueprint(const FString& BlueprintPath)
    {
        UObject* Asset = LoadObject<UObject>(nullptr, *BlueprintPath);
        if (!Asset)
        {
            return nullptr;
        }
        
        UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
        if (!Blueprint)
        {
            Blueprint = Cast<UBlueprint>(Asset->GetClass()->ClassGeneratedBy);
        }
        
        return Blueprint;
    }

    void ValidateVariables(UBlueprint* Blueprint, TArray<TSharedPtr<FJsonValue>>& OutIssues, TArray<TSharedPtr<FJsonValue>>& OutWarnings)
    {
        TSet<FName> VariableNames;

        for (int32 i = 0; i < Blueprint->NewVariables.Num(); ++i)
        {
            const FBPVariableDescription& Var = Blueprint->NewVariables[i];

            TSharedPtr<FJsonObject> Issue = nullptr;

            if (Var.VarName.IsNone() || Var.VarName.ToString().IsEmpty())
            {
                Issue = MakeShareable(new FJsonObject);
                Issue->SetStringField(TEXT("type"), TEXT("empty_variable_name"));
                Issue->SetStringField(TEXT("severity"), TEXT("error"));
                Issue->SetStringField(TEXT("message"), TEXT("Variable has empty name"));
                Issue->SetNumberField(TEXT("variable_index"), i);
                OutIssues.Add(MakeShareable(new FJsonValueObject(Issue)));
            }
            else if (VariableNames.Contains(Var.VarName))
            {
                Issue = MakeShareable(new FJsonObject);
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

    void ValidateFunctions(UBlueprint* Blueprint, TArray<TSharedPtr<FJsonValue>>& OutIssues, TArray<TSharedPtr<FJsonValue>>& OutWarnings)
    {
        TSet<FName> FunctionNames;

        for (UEdGraph* Graph : Blueprint->FunctionGraphs)
        {
            if (!Graph)
            {
                continue;
            }

            FString GraphName = Graph->GetName();

            if (FunctionNames.Contains(*GraphName))
            {
                TSharedPtr<FJsonObject> Issue = MakeShareable(new FJsonObject);
                Issue->SetStringField(TEXT("type"), TEXT("duplicate_function_name"));
                Issue->SetStringField(TEXT("severity"), TEXT("error"));
                Issue->SetStringField(TEXT("message"), FString::Printf(TEXT("Duplicate function name: %s"), *GraphName));
                Issue->SetStringField(TEXT("graph_name"), GraphName);
                OutIssues.Add(MakeShareable(new FJsonValueObject(Issue)));
            }
            else
            {
                FunctionNames.Add(*GraphName);
            }

            int32 NodeCount = Graph->Nodes.Num();
            if (NodeCount == 0)
            {
                TSharedPtr<FJsonObject> Warning = MakeShareable(new FJsonObject);
                Warning->SetStringField(TEXT("type"), TEXT("empty_function"));
                Warning->SetStringField(TEXT("severity"), TEXT("warning"));
                Warning->SetStringField(TEXT("message"), FString::Printf(TEXT("Function '%s' is empty"), *GraphName));
                Warning->SetStringField(TEXT("graph_name"), GraphName);
                OutWarnings.Add(MakeShareable(new FJsonValueObject(Warning)));
            }
        }
    }

    void ValidateConnections(UBlueprint* Blueprint, TArray<TSharedPtr<FJsonValue>>& OutIssues, TArray<TSharedPtr<FJsonValue>>& OutWarnings)
    {
        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(Blueprint->UbergraphPages);
        AllGraphs.Append(Blueprint->FunctionGraphs);
        AllGraphs.Append(Blueprint->MacroGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph)
            {
                continue;
            }

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (!Node)
                {
                    continue;
                }

                for (UEdGraphPin* Pin : Node->Pins)
                {
                    if (!Pin)
                    {
                        continue;
                    }

                    if (Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() == 0)
                    {
                        if (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec &&
                            !Pin->PinName.ToString().StartsWith(TEXT("self")) &&
                            !Pin->PinName.ToString().StartsWith(TEXT("WorldContext")))
                        {
                            FString DefaultValue = Pin->DefaultValue;
                            FString AutogeneratedDefaultValue = Pin->AutogeneratedDefaultValue;
                            
                            if (DefaultValue.IsEmpty() && AutogeneratedDefaultValue.IsEmpty())
                            {
                                TSharedPtr<FJsonObject> Warning = MakeShareable(new FJsonObject);
                                Warning->SetStringField(TEXT("type"), TEXT("unconnected_input_pin"));
                                Warning->SetStringField(TEXT("severity"), TEXT("warning"));
                                Warning->SetStringField(TEXT("message"), FString::Printf(
                                    TEXT("Unconnected input pin '%s' on node '%s' in graph '%s'"),
                                    *Pin->PinName.ToString(),
                                    *Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString(),
                                    *Graph->GetName()
                                ));
                                Warning->SetStringField(TEXT("graph_name"), Graph->GetName());
                                Warning->SetStringField(TEXT("node_name"), Node->GetName());
                                Warning->SetStringField(TEXT("pin_name"), Pin->PinName.ToString());
                                OutWarnings.Add(MakeShareable(new FJsonValueObject(Warning)));
                            }
                        }
                    }
                }
            }
        }
    }
};

REGISTER_MCP_COMMAND(FMcpValidateBlueprintHandler)
