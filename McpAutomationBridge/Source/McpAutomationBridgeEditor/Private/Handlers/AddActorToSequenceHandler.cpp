#include "CoreMinimal.h"
#include "McpCommand.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "MovieScene.h"
#include "MovieSceneTrack.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "GameFramework/Actor.h"
#include "CineCameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "EngineUtils.h"

class FMcpAddActorToSequenceHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_actor_to_sequence"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SequencePath;
        if (!Params->TryGetStringField(TEXT("sequence_path"), SequencePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'sequence_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ActorName;
        if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_name' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bPossessable = true;
        Params->TryGetBoolField(TEXT("possessable"), bPossessable);

        bool bAddTransformTrack = true;
        Params->TryGetBoolField(TEXT("add_transform_track"), bAddTransformTrack);

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
        AActor* FoundActor = nullptr;

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (World)
        {
            for (TActorIterator<AActor> It(World); It; ++It)
            {
                if (It->GetActorLabel() == ActorName || It->GetName() == ActorName)
                {
                    FoundActor = *It;
                    break;
                }
            }
        }

        if (!FoundActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorName),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        if (bPossessable)
        {
            BindingGuid = MovieScene->AddPossessable(ActorName, FoundActor->GetClass());
        }
        else
        {
            BindingGuid = MovieScene->AddSpawnable(ActorName, *FoundActor);
        }

        if (!BindingGuid.IsValid())
        {
            return FMcpCommandResult::Failure(TEXT("Failed to add actor to sequence"), TEXT("BINDING_FAILED"));
        }

        if (bAddTransformTrack)
        {
            MovieScene->AddTrack<UMovieScene3DTransformTrack>(BindingGuid);
        }

        Sequence->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("added"), true);
        Result->SetStringField(TEXT("actor_name"), ActorName);
        Result->SetStringField(TEXT("binding_guid"), BindingGuid.ToString());
        Result->SetBoolField(TEXT("possessable"), bPossessable);
        Result->SetBoolField(TEXT("transform_track_added"), bAddTransformTrack);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpAddActorToSequenceHandler)
