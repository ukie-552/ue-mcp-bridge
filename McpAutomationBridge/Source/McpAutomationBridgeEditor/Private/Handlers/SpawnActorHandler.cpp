#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "EditorLevelLibrary.h"
#include "Subsystems/EditorActorSubsystem.h"

class FMcpSpawnActorHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("spawn_actor"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorClassPath;
        if (!Params->TryGetStringField(TEXT("actor_class"), ActorClassPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_class' parameter"), TEXT("MISSING_PARAM"));
        }

        FVector Location = FVector::ZeroVector;
        const TSharedPtr<FJsonObject>* LocationObj;
        if (Params->TryGetObjectField(TEXT("location"), LocationObj))
        {
            double X = 0, Y = 0, Z = 0;
            (*LocationObj)->TryGetNumberField(TEXT("x"), X);
            (*LocationObj)->TryGetNumberField(TEXT("y"), Y);
            (*LocationObj)->TryGetNumberField(TEXT("z"), Z);
            Location = FVector(X, Y, Z);
        }

        FRotator Rotation = FRotator::ZeroRotator;
        const TSharedPtr<FJsonObject>* RotationObj;
        if (Params->TryGetObjectField(TEXT("rotation"), RotationObj))
        {
            double Pitch = 0, Yaw = 0, Roll = 0;
            (*RotationObj)->TryGetNumberField(TEXT("pitch"), Pitch);
            (*RotationObj)->TryGetNumberField(TEXT("yaw"), Yaw);
            (*RotationObj)->TryGetNumberField(TEXT("roll"), Roll);
            Rotation = FRotator(Pitch, Yaw, Roll);
        }

        FString ActorName;
        Params->TryGetStringField(TEXT("name"), ActorName);

        UClass* ActorClass = LoadClass<AActor>(nullptr, *ActorClassPath);
        if (!ActorClass)
        {
            UObject* Asset = LoadObject<UObject>(nullptr, *ActorClassPath);
            if (Asset)
            {
                ActorClass = Cast<UClass>(Asset);
            }
        }

        if (!ActorClass)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor class not found: %s"), *ActorClassPath),
                TEXT("CLASS_NOT_FOUND")
            );
        }

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            return FMcpCommandResult::Failure(TEXT("No editor world available"), TEXT("NO_WORLD"));
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
        if (!NewActor)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to spawn actor"), TEXT("SPAWN_FAILED"));
        }

        if (!ActorName.IsEmpty())
        {
            NewActor->SetActorLabel(*ActorName);
        }

        NewActor->RerunConstructionScripts();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("actor_id"), NewActor->GetActorGuid().ToString());
        Result->SetStringField(TEXT("actor_name"), NewActor->GetActorLabel());
        Result->SetStringField(TEXT("path"), NewActor->GetPathName());

        TSharedPtr<FJsonObject> LocJson = MakeShareable(new FJsonObject);
        LocJson->SetNumberField(TEXT("x"), NewActor->GetActorLocation().X);
        LocJson->SetNumberField(TEXT("y"), NewActor->GetActorLocation().Y);
        LocJson->SetNumberField(TEXT("z"), NewActor->GetActorLocation().Z);
        Result->SetObjectField(TEXT("location"), LocJson);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSpawnActorHandler)
