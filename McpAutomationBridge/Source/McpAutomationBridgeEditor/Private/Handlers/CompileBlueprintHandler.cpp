#include "CoreMinimal.h"
#include "McpCommand.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Blueprint.h"
#include "Blueprint/UserWidget.h"

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
            
            FString SourceLocation;
            if (Message->SourceLocation.IsValid())
            {
                SourceLocation = FString::Printf(
                    TEXT("%s:%d"),
                    *Message->SourceLocation.FileName.ToString(),
                    Message->SourceLocation.LineNumber
                );
            }
            MsgJson->SetStringField(TEXT("location"), SourceLocation);

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

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("success"), !MessageLog.HasErrors());
        Result->SetNumberField(TEXT("error_count"), ErrorsArray.Num());
        Result->SetNumberField(TEXT("warning_count"), WarningsArray.Num());
        Result->SetNumberField(TEXT("note_count"), NotesArray.Num());
        Result->SetArrayField(TEXT("errors"), ErrorsArray);
        Result->SetArrayField(TEXT("warnings"), WarningsArray);
        Result->SetArrayField(TEXT("notes"), NotesArray);

        if (!MessageLog.HasErrors() && bSaveOnSuccess)
        {
            FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
            UPackage* Package = Blueprint->GetOutermost();
            if (Package)
            {
                FString PackagePath = Package->GetName();
                UPackage::SavePackage(Package, Blueprint, RF_Public | RF_Standalone, *PackagePath);
                Result->SetBoolField(TEXT("saved"), true);
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
