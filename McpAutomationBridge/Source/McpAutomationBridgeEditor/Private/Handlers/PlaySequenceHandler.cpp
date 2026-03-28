#include "CoreMinimal.h"
#include "McpCommand.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "MovieScene.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

class FMcpPlaySequenceHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("play_sequence"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SequencePath;
        if (!Params->TryGetStringField(TEXT("sequence_path"), SequencePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'sequence_path' parameter"), TEXT("MISSING_PARAM"));
        }

        double StartTime = 0.0;
        Params->TryGetNumberField(TEXT("start_time"), StartTime);

        bool bLoop = false;
        Params->TryGetBoolField(TEXT("loop"), bLoop);

        ULevelSequence* Sequence = LoadObject<ULevelSequence>(nullptr, *SequencePath);
        if (!Sequence)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Level sequence not found: %s"), *SequencePath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            return FMcpCommandResult::Failure(TEXT("No editor world available"), TEXT("NO_WORLD"));
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ALevelSequenceActor* SequenceActor = World->SpawnActor<ALevelSequenceActor>(ALevelSequenceActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (!SequenceActor)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to spawn level sequence actor"), TEXT("SPAWN_FAILED"));
        }

        SequenceActor->SetSequence(Sequence);

        FMovieSceneSequencePlaybackSettings PlaybackSettings;
        PlaybackSettings.bAutoPlay = true;
        PlaybackSettings.LoopCount.Value = bLoop ? 0 : 1;
        PlaybackSettings.StartTime = StartTime;

        SequenceActor->PlaybackSettings = PlaybackSettings;

        SequenceActor->GetSequencePlayer()->Play();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("playing"), true);
        Result->SetStringField(TEXT("sequence_name"), Sequence->GetName());
        Result->SetNumberField(TEXT("start_time"), StartTime);
        Result->SetBoolField(TEXT("loop"), bLoop);
        Result->SetStringField(TEXT("sequence_actor"), SequenceActor->GetName());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpPlaySequenceHandler)
