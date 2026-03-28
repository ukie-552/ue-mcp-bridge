#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpDeleteBlueprintVariableHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("delete_blueprint_variable"); }

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

        UBlueprint* Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        int32 VarIndex = INDEX_NONE;
        for (int32 i = 0; i < Blueprint->NewVariables.Num(); i++)
        {
            if (Blueprint->NewVariables[i].VarName == *VariableName)
            {
                VarIndex = i;
                break;
            }
        }

        if (VarIndex == INDEX_NONE)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Variable not found: %s"), *VariableName),
                TEXT("VARIABLE_NOT_FOUND")
            );
        }

        Blueprint->NewVariables.RemoveAt(VarIndex);
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("deleted"), true);
        Result->SetStringField(TEXT("variable_name"), VariableName);
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpDeleteBlueprintVariableHandler)
