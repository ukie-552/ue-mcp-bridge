#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "EditorLevelLibrary.h"
#include "FileHelpers.h"
#include "LevelEditor.h"

class FMcpLoadLevelHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("load_level"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString LevelPath;
        if (!Params->TryGetStringField(TEXT("level_path"), LevelPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'level_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bSaveCurrent = true;
        Params->TryGetBoolField(TEXT("save_current"), bSaveCurrent);

        if (bSaveCurrent)
        {
            UWorld* CurrentWorld = GEditor->GetEditorWorldContext().World();
            if (CurrentWorld)
            {
                FEditorFileUtils::SaveCurrentLevel();
            }
        }

        FString FullPath = LevelPath;
        if (!FullPath.EndsWith(TEXT(".umap")))
        {
            FullPath += TEXT(".umap");
        }

        bool bLoaded = FEditorFileUtils::LoadMap(*FullPath);

        if (!bLoaded)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to load level: %s"), *LevelPath),
                TEXT("LEVEL_LOAD_FAILED")
            );
        }

        UWorld* LoadedWorld = GEditor->GetEditorWorldContext().World();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("loaded"), true);
        Result->SetStringField(TEXT("level_path"), LevelPath);
        if (LoadedWorld)
        {
            Result->SetStringField(TEXT("level_name"), LoadedWorld->GetName());
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpLoadLevelHandler)
