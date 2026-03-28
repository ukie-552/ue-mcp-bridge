#include "CoreMinimal.h"
#include "McpCommand.h"
#include "EditorLevelLibrary.h"
#include "FileHelpers.h"

class FMcpSaveCurrentLevelHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("save_current_level"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            return FMcpCommandResult::Failure(TEXT("No level currently open"), TEXT("NO_LEVEL"));
        }

        FString LevelName = World->GetName();

        bool bSaved = FEditorFileUtils::SaveCurrentLevel();

        if (!bSaved)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to save level: %s"), *LevelName),
                TEXT("SAVE_FAILED")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("saved"), true);
        Result->SetStringField(TEXT("level_name"), LevelName);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSaveCurrentLevelHandler)
