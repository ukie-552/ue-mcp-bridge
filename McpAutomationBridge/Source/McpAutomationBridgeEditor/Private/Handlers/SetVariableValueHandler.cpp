#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"

class FMcpSetVariableValueHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_variable_value"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString VariableName;
        if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'variable_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString Value;
        if (!Params->TryGetStringField(TEXT("value"), Value))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'value' parameter"), TEXT("MISSING_PARAM"));
        }

        UBlueprint* Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        FBPVariableDescription* VarDesc = nullptr;
        for (FBPVariableDescription& Var : Blueprint->NewVariables)
        {
            if (Var.VarName == *VariableName)
            {
                VarDesc = &Var;
                break;
            }
        }

        if (!VarDesc)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Variable not found: %s"), *VariableName),
                TEXT("VARIABLE_NOT_FOUND")
            );
        }

        VarDesc->DefaultValue = Value;
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), true);
        Result->SetStringField(TEXT("variable_name"), VariableName);
        Result->SetStringField(TEXT("value"), Value);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSetVariableValueHandler)
