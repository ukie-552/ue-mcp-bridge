#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpDeleteAnimBlueprintVariableHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("delete_anim_blueprint_variable"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString VariableName;
        if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'variable_name' parameter"), TEXT("MISSING_PARAM"));
        }

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        int32 VarIndex = INDEX_NONE;
        for (int32 i = 0; i < AnimBP->NewVariables.Num(); i++)
        {
            if (AnimBP->NewVariables[i].VarName == *VariableName)
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

        AnimBP->NewVariables.RemoveAt(VarIndex);
        FBlueprintEditorUtils::MarkBlueprintAsModified(AnimBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("deleted"), true);
        Result->SetStringField(TEXT("variable_name"), VariableName);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpDeleteAnimBlueprintVariableHandler)