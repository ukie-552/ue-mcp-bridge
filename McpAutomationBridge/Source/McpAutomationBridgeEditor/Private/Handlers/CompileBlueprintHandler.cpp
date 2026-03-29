#include "CoreMinimal.h"
#include "McpCommand.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Blueprint.h"
#include "Blueprint/UserWidget.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "UObject/SavePackage.h"

class FMcpCompileBlueprintHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("compile_blueprint"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bSaveOnSuccess = false;
        Params->TryGetBoolField(TEXT("save_on_success"), bSaveOnSuccess);

        UBlueprint* Blueprint = LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        FCompilerResultsLog MessageLog;
        FKismetCompilerOptions CompileOptions;
        CompileOptions.bSaveIntermediateProducts = false;

        TSharedPtr<FKismetCompilerContext> Compiler = 
            FKismetCompilerContext::GetCompilerForBP(Blueprint, MessageLog, CompileOptions);

        if (!Compiler.IsValid())
        {
            return FMcpCommandResult::Failure(
                TEXT("Failed to create compiler context"),
                TEXT("COMPILER_ERROR")
            );
        }

        Compiler->Compile();

        TArray<TSharedPtr<FJsonValue>> ErrorsArray;
        TArray<TSharedPtr<FJsonValue>> WarningsArray;
        TArray<TSharedPtr<FJsonValue>> NotesArray;

        for (const TSharedRef<FTokenizedMessage>& Message : MessageLog.Messages)
        {
            TSharedPtr<FJsonObject> MsgJson = MakeShareable(new FJsonObject);
            MsgJson->SetStringField(TEXT("message"), Message->ToText().ToString());
            MsgJson->SetStringField(TEXT("location"), TEXT(""));

            switch (Message->GetSeverity())
            {
                case EMessageSeverity::Error:
                    ErrorsArray.Add(MakeShareable(new FJsonValueObject(MsgJson)));
                    break;
                case EMessageSeverity::Warning:
                case EMessageSeverity::PerformanceWarning:
                    WarningsArray.Add(MakeShareable(new FJsonValueObject(MsgJson)));
                    break;
                default:
                    NotesArray.Add(MakeShareable(new FJsonValueObject(MsgJson)));
                    break;
            }
        }

        bool bHasErrors = MessageLog.NumErrors > 0;

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("success"), !bHasErrors);
        Result->SetNumberField(TEXT("error_count"), ErrorsArray.Num());
        Result->SetNumberField(TEXT("warning_count"), WarningsArray.Num());
        Result->SetNumberField(TEXT("note_count"), NotesArray.Num());
        Result->SetArrayField(TEXT("errors"), ErrorsArray);
        Result->SetArrayField(TEXT("warnings"), WarningsArray);
        Result->SetArrayField(TEXT("notes"), NotesArray);

        if (!bHasErrors && bSaveOnSuccess)
        {
            FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
            UPackage* Package = Blueprint->GetOutermost();
            if (Package)
            {
                FString PackagePath = Package->GetName();
                FString Filename;
                if (FPackageName::TryConvertLongPackageNameToFilename(PackagePath, Filename))
                {
                    Filename += FPackageName::GetAssetPackageExtension();
                    FSavePackageArgs SaveArgs;
                    UPackage::SavePackage(Package, Blueprint, *Filename, SaveArgs);
                    Result->SetBoolField(TEXT("saved"), true);
                }
            }
        }

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
};

REGISTER_MCP_COMMAND(FMcpCompileBlueprintHandler)
