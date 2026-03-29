#include "CoreMinimal.h"
#include "McpCommand.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "MovieScene.h"
#include "MovieSceneTrack.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "Tracks/MovieSceneAudioTrack.h"
#include "Tracks/MovieSceneFadeTrack.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Sections/MovieSceneCameraCutSection.h"
#include "GameFramework/Actor.h"
#include "CineCameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"

class FMcpAddSequenceTrackHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_sequence_track"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SequencePath;
        if (!Params->TryGetStringField(TEXT("sequence_path"), SequencePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'sequence_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString TrackType;
        if (!Params->TryGetStringField(TEXT("track_type"), TrackType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'track_type' parameter"), TEXT("MISSING_PARAM"));
        }

        FString BindingName;
        Params->TryGetStringField(TEXT("binding_name"), BindingName);

        FString ActorPath;
        Params->TryGetStringField(TEXT("actor_path"), ActorPath);

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

        FGuid BindingGuid;

        if (!BindingName.IsEmpty())
        {
            const TArrayView<const FMovieSceneBinding> Bindings = MovieScene->GetBindings();
            for (const FMovieSceneBinding& Binding : Bindings)
            {
                FString BindingNameStr;
                if (const FMovieSceneSpawnable* Spawnable = MovieScene->FindSpawnable(Binding.GetObjectGuid()))
                {
                    BindingNameStr = Spawnable->GetName();
                }
                else if (const FMovieScenePossessable* Possessable = MovieScene->FindPossessable(Binding.GetObjectGuid()))
                {
                    BindingNameStr = Possessable->GetName();
                }

                if (BindingNameStr == BindingName)
                {
                    BindingGuid = Binding.GetObjectGuid();
                    break;
                }
            }

            if (!BindingGuid.IsValid())
            {
                BindingGuid = FGuid::NewGuid();
            }
        }

        UMovieSceneTrack* NewTrack = CreateTrackByType(MovieScene, TrackType, BindingGuid);
        if (!NewTrack)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create track of type: %s"), *TrackType),
                TEXT("TRACK_CREATION_FAILED")
            );
        }

        Sequence->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("track_type"), TrackType);
        Result->SetStringField(TEXT("track_name"), NewTrack->GetTrackName().ToString());

        if (BindingGuid.IsValid())
        {
            Result->SetStringField(TEXT("binding_guid"), BindingGuid.ToString());
            Result->SetStringField(TEXT("binding_name"), BindingName);
        }

        return FMcpCommandResult::Success(Result);
    }

private:
    UMovieSceneTrack* CreateTrackByType(UMovieScene* MovieScene, const FString& TrackType, const FGuid& BindingGuid)
    {
        if (!MovieScene)
        {
            return nullptr;
        }

        UMovieSceneTrack* NewTrack = nullptr;

        if (TrackType == TEXT("Transform") || TrackType == TEXT("3DTransform"))
        {
            if (BindingGuid.IsValid())
            {
                NewTrack = MovieScene->AddTrack<UMovieScene3DTransformTrack>(BindingGuid);
            }
        }
        else if (TrackType == TEXT("CameraCut"))
        {
            NewTrack = MovieScene->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass());
        }
        else if (TrackType == TEXT("Audio"))
        {
            NewTrack = MovieScene->AddTrack<UMovieSceneAudioTrack>();
        }
        else if (TrackType == TEXT("Fade"))
        {
            NewTrack = MovieScene->AddTrack<UMovieSceneFadeTrack>();
        }
        else
        {
            UClass* TrackClass = LoadClass<UMovieSceneTrack>(nullptr, *TrackType);
            if (!TrackClass)
            {
                FString FullClassName = FString::Printf(TEXT("Engine.%s"), *TrackType);
                TrackClass = LoadClass<UMovieSceneTrack>(nullptr, *FullClassName);
            }

            if (TrackClass)
            {
                if (BindingGuid.IsValid())
                {
                    NewTrack = MovieScene->AddTrack(TrackClass, BindingGuid);
                }
                else
                {
                    NewTrack = MovieScene->AddTrack(TrackClass);
                }
            }
        }

        return NewTrack;
    }
};

REGISTER_MCP_COMMAND(FMcpAddSequenceTrackHandler)
