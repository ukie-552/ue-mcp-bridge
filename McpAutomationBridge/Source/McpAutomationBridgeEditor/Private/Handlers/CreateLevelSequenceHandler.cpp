#include "CoreMinimal.h"
#include "McpCommand.h"
#include "LevelSequence.h"
#include "MovieScene.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

class FMcpCreateLevelSequenceHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_level_sequence"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SequencePath;
        if (!Params->TryGetStringField(TEXT("sequence_path"), SequencePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'sequence_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString SequenceName;
        if (!Params->TryGetStringField(TEXT("sequence_name"), SequenceName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'sequence_name' parameter"), TEXT("MISSING_PARAM"));
        }

        double Duration = 5.0;
        Params->TryGetNumberField(TEXT("duration"), Duration);

        int32 FrameRate = 30;
        Params->TryGetNumberField(TEXT("frame_rate"), FrameRate);

        FString FullPath = FString::Printf(TEXT("%s/%s"), *SequencePath, *SequenceName);

        FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
        
        UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
            SequenceName,
            SequencePath,
            ULevelSequence::StaticClass(),
            nullptr
        );

        if (!NewAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create level sequence: %s"), *SequenceName),
                TEXT("SEQUENCE_CREATION_FAILED")
            );
        }

        ULevelSequence* NewSequence = Cast<ULevelSequence>(NewAsset);
        if (NewSequence)
        {
            UMovieScene* MovieScene = NewSequence->GetMovieScene();
            if (MovieScene)
            {
                FFrameRate DisplayRate(FrameRate, 1);
                MovieScene->SetDisplayRate(DisplayRate);

                FFrameNumber EndFrame = FFrameNumber(static_cast<int32>(Duration * FrameRate));
                MovieScene->SetPlaybackRange(TRange<FFrameNumber>(0, EndFrame));
            }

            FAssetRegistryModule::AssetCreated(NewSequence);
            NewSequence->MarkPackageDirty();
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("sequence_name"), SequenceName);
        Result->SetStringField(TEXT("sequence_path"), FullPath);
        Result->SetNumberField(TEXT("duration"), Duration);
        Result->SetNumberField(TEXT("frame_rate"), FrameRate);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateLevelSequenceHandler)
