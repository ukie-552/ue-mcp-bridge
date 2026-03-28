#include "CoreMinimal.h"
#include "McpCommand.h"
#include "LevelSequence.h"
#include "MovieScene.h"
#include "MovieSceneTrack.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "Tracks/MovieSceneAudioTrack.h"

class FMcpGetSequenceInfoHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_sequence_info"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SequencePath;
        if (!Params->TryGetStringField(TEXT("sequence_path"), SequencePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'sequence_path' parameter"), TEXT("MISSING_PARAM"));
        }

        ULevelSequence* Sequence = LoadObject<ULevelSequence>(nullptr, *SequencePath);
        if (!Sequence)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Level sequence not found: %s"), *SequencePath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UMovieScene* MovieScene = Sequence->GetMovieScene();
        if (!MovieScene)
        {
            return FMcpCommandResult::Failure(TEXT("Movie scene not found"), TEXT("NO_MOVIE_SCENE"));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("sequence_name"), Sequence->GetName());
        Result->SetStringField(TEXT("sequence_path"), SequencePath);

        FFrameRate DisplayRate = Sequence->GetDisplayRate();
        Result->SetNumberField(TEXT("frame_rate"), DisplayRate.AsDecimal());

        TRange<FFrameNumber> PlaybackRange = MovieScene->GetPlaybackRange();
        double Duration = 0.0;
        if (PlaybackRange.HasUpperBound())
        {
            Duration = PlaybackRange.GetUpperBoundValue().Value / static_cast<double>(DisplayRate.AsDecimal());
        }
        Result->SetNumberField(TEXT("duration"), Duration);

        TArray<TSharedPtr<FJsonValue>> BindingsArray;
        for (const FMovieSceneBinding& Binding : MovieScene->GetBindings())
        {
            TSharedPtr<FJsonObject> BindingInfo = MakeShareable(new FJsonObject);
            BindingInfo->SetStringField(TEXT("name"), Binding.GetName());
            BindingInfo->SetStringField(TEXT("guid"), Binding.GetObjectGuid().ToString());

            TArray<TSharedPtr<FJsonValue>> TracksArray;
            for (UMovieSceneTrack* Track : Binding.GetTracks())
            {
                if (Track)
                {
                    TSharedPtr<FJsonObject> TrackInfo = MakeShareable(new FJsonObject);
                    TrackInfo->SetStringField(TEXT("name"), Track->GetTrackName().ToString());
                    TrackInfo->SetStringField(TEXT("type"), Track->GetClass()->GetName());
                    TrackInfo->SetNumberField(TEXT("section_count"), Track->GetAllSections().Num());
                    TracksArray.Add(MakeShareable(new FJsonValueObject(TrackInfo)));
                }
            }
            BindingInfo->SetArrayField(TEXT("tracks"), TracksArray);

            BindingsArray.Add(MakeShareable(new FJsonValueObject(BindingInfo)));
        }
        Result->SetArrayField(TEXT("bindings"), BindingsArray);
        Result->SetNumberField(TEXT("binding_count"), BindingsArray.Num());

        TArray<TSharedPtr<FJsonValue>> MasterTracksArray;
        for (UMovieSceneTrack* MasterTrack : MovieScene->GetMasterTracks())
        {
            if (MasterTrack)
            {
                TSharedPtr<FJsonObject> TrackInfo = MakeShareable(new FJsonObject);
                TrackInfo->SetStringField(TEXT("name"), MasterTrack->GetTrackName().ToString());
                TrackInfo->SetStringField(TEXT("type"), MasterTrack->GetClass()->GetName());
                TrackInfo->SetNumberField(TEXT("section_count"), MasterTrack->GetAllSections().Num());
                MasterTracksArray.Add(MakeShareable(new FJsonValueObject(TrackInfo)));
            }
        }
        Result->SetArrayField(TEXT("master_tracks"), MasterTracksArray);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetSequenceInfoHandler)
