#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Editor.h"
#include "Subsystems/EditorActorSubsystem.h"

class FMcpDuplicateActorHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("duplicate_actor"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        AActor* SourceActor = FindActorById(ActorId);
        if (!SourceActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        FString NewName;
        Params->TryGetStringField(TEXT("new_name"), NewName);

        FVector OffsetLocation = FVector::ZeroVector;
        const TSharedPtr<FJsonObject>* OffsetObj;
        if (Params->TryGetObjectField(TEXT("offset"), OffsetObj))
        {
            double X = 0, Y = 0, Z = 0;
            (*OffsetObj)->TryGetNumberField(TEXT("x"), X);
            (*OffsetObj)->TryGetNumberField(TEXT("y"), Y);
            (*OffsetObj)->TryGetNumberField(TEXT("z"), Z);
            OffsetLocation = FVector(X, Y, Z);
        }

        FVector NewLocation = SourceActor->GetActorLocation() + OffsetLocation;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Template = SourceActor;
        if (!NewName.IsEmpty())
        {
            SpawnParams.Name = *NewName;
        }

        UWorld* World = GEditor->GetEditorWorldContext().World();
        FRotator SourceRotation = SourceActor->GetActorRotation();
        AActor* NewActor = World->SpawnActor(SourceActor->GetClass(), &NewLocation, &SourceRotation, SpawnParams);

        if (!NewActor)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to duplicate actor"), TEXT("DUPLICATION_FAILED"));
        }

        if (!NewName.IsEmpty())
        {
            NewActor->SetActorLabel(*NewName);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("duplicated"), true);
        Result->SetStringField(TEXT("source_actor_id"), ActorId);
        Result->SetStringField(TEXT("new_actor_id"), NewActor->GetActorGuid().ToString());
        Result->SetStringField(TEXT("new_actor_name"), NewActor->GetActorLabel());

        TSharedPtr<FJsonObject> LocJson = MakeShareable(new FJsonObject);
        LocJson->SetNumberField(TEXT("x"), NewActor->GetActorLocation().X);
        LocJson->SetNumberField(TEXT("y"), NewActor->GetActorLocation().Y);
        LocJson->SetNumberField(TEXT("z"), NewActor->GetActorLocation().Z);
        Result->SetObjectField(TEXT("location"), LocJson);

        return FMcpCommandResult::Success(Result);
    }

private:
    AActor* FindActorById(const FString& ActorId)
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World) return nullptr;

        FGuid TargetGuid;
        if (!FGuid::Parse(ActorId, TargetGuid)) return nullptr;

        for (TActorIterator<AActor> It(World); It; ++It)
        {
            if ((*It)->GetActorGuid() == TargetGuid)
            {
                return *It;
            }
        }

        return nullptr;
    }
};

REGISTER_MCP_COMMAND(FMcpDuplicateActorHandler)
