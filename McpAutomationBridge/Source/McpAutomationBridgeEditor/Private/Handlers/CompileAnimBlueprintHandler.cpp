#include "CoreMinimal.h"
#include "McpCommand.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Animation/AnimBlueprint.h"
#include "BlueprintEditor.h"

class FMcpCompileAnimBlueprintHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("compile_anim_blueprint"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        FCompilerResultsLog MessageLog;
        FKismetCompilerOptions CompileOptions;

        TArray<FCompilerResultLogEntry> Messages;

        bool bCompileSuccess = true;

        FBlueprintEditorUtils::CompileBlueprint(AnimBP, false);

        bCompileSuccess = !AnimBP->bHasErrors;

        if (AnimBP->bHasErrors)
        {
            bCompileSuccess = false;
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("success"), bCompileSuccess);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetStringField(TEXT("anim_blueprint_name"), AnimBP->GetName());

        Result->SetBoolField(TEXT("has_errors"), AnimBP->bHasErrors);
        Result->SetBoolField(TEXT("is_skeleton_only"), AnimBP->bIsSkeletonOnly);

        if (AnimBP->Status == EBlueprintStatus::BS_Error)
        {
            Result->SetStringField(TEXT("status"), TEXT("Error"));
        }
        else if (AnimBP->Status == EBlueprintStatus::BS_UpToDate)
        {
            Result->SetStringField(TEXT("status"), TEXT("UpToDate"));
        }
        else if (AnimBP->Status == EBlueprintStatus::BS_Dirty)
        {
            Result->SetStringField(TEXT("status"), TEXT("Dirty"));
        }
        else if (AnimBP->Status == EBlueprintStatus::BS_Unknown)
        {
            Result->SetStringField(TEXT("status"), TEXT("Unknown"));
        }
        else
        {
            Result->SetStringField(TEXT("status"), TEXT("Other"));
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCompileAnimBlueprintHandler)