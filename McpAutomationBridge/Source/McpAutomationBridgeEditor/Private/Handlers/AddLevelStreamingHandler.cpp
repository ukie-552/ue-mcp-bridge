#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "EditorLevelLibrary.h"
#include "LevelEditor.h"

class FMcpAddLevelStreamingHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_level_streaming"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString LevelPath;
        if (!Params->TryGetStringField(TEXT("level_path"), LevelPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'level_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString StreamingMethod = TEXT("AlwaysLoaded");
        Params->TryGetStringField(TEXT("streaming_method"), StreamingMethod);

        FString LevelName;
        Params->TryGetStringField(TEXT("level_name"), LevelName);

        if (LevelName.IsEmpty())
        {
            LevelName = FPaths::GetBaseFilename(LevelPath);
        }

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            return FMcpCommandResult::Failure(TEXT("No world available"), TEXT("NO_WORLD"));
        }

        FString PackageName = LevelPath;
        if (!PackageName.StartsWith(TEXT("/")))
        {
            PackageName = FString::Printf(TEXT("/Game/Maps/%s"), *PackageName);
        }

        ULevelStreaming* StreamingLevel = nullptr;

        if (StreamingMethod == TEXT("AlwaysLoaded"))
        {
            StreamingLevel = GEditor->GetEditorWorldContext().AddStreamingLevel(World, *PackageName, FVector::ZeroVector, FRotator::ZeroRotator);
        }
        else
        {
            StreamingLevel = NewObject<ULevelStreaming>(World);
            if (StreamingLevel)
            {
                StreamingLevel->SetWorldAssetByPackageName(*PackageName);
                StreamingLevel->SetShouldBeLoaded(true);
                StreamingLevel->SetShouldBeVisible(StreamingMethod == TEXT("Visible"));
                
                World->AddStreamingLevel(StreamingLevel);
            }
        }

        if (!StreamingLevel)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to add streaming level: %s"), *LevelPath),
                TEXT("STREAMING_FAILED")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("added"), true);
        Result->SetStringField(TEXT("level_path"), LevelPath);
        Result->SetStringField(TEXT("level_name"), LevelName);
        Result->SetStringField(TEXT("streaming_method"), StreamingMethod);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpAddLevelStreamingHandler)
