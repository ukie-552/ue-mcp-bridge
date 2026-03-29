#include "CoreMinimal.h"
#include "McpCommand.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraActor.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Editor.h"
#include "Engine/World.h"

class FMcpSpawnNiagaraActorHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("spawn_niagara_actor"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SystemPath;
        if (!Params->TryGetStringField(TEXT("system_path"), SystemPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'system_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ActorName;
        Params->TryGetStringField(TEXT("actor_name"), ActorName);

        if (ActorName.IsEmpty())
        {
            ActorName = TEXT("NiagaraActor");
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

        FVector Scale = FVector::OneVector;
        const TSharedPtr<FJsonObject>* ScaleObj;
        if (Params->TryGetObjectField(TEXT("scale"), ScaleObj))
        {
            double X = 1, Y = 1, Z = 1;
            (*ScaleObj)->TryGetNumberField(TEXT("x"), X);
            (*ScaleObj)->TryGetNumberField(TEXT("y"), Y);
            (*ScaleObj)->TryGetNumberField(TEXT("z"), Z);
            Scale = FVector(X, Y, Z);
        }

        bool bAutoActivate = true;
        Params->TryGetBoolField(TEXT("auto_activate"), bAutoActivate);

        bool bAutoDestroy = false;
        Params->TryGetBoolField(TEXT("auto_destroy"), bAutoDestroy);

        UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
        if (!NiagaraSystem)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath),
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
        SpawnParams.Name = *ActorName;

        ANiagaraActor* NiagaraActor = World->SpawnActor<ANiagaraActor>(ANiagaraActor::StaticClass(), Location, Rotation, SpawnParams);
        if (!NiagaraActor)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to spawn Niagara actor"), TEXT("SPAWN_FAILED"));
        }

        NiagaraActor->SetActorLabel(*ActorName);
        NiagaraActor->SetActorScale3D(Scale);

        UNiagaraComponent* NiagaraComponent = NiagaraActor->GetNiagaraComponent();
        if (NiagaraComponent)
        {
            NiagaraComponent->SetAsset(NiagaraSystem);
            NiagaraComponent->SetAutoActivate(bAutoActivate);
            NiagaraComponent->SetAutoDestroy(bAutoDestroy);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("spawned"), true);
        Result->SetStringField(TEXT("actor_name"), NiagaraActor->GetActorLabel());
        Result->SetStringField(TEXT("system_path"), SystemPath);
        Result->SetBoolField(TEXT("auto_activate"), bAutoActivate);
        Result->SetBoolField(TEXT("auto_destroy"), bAutoDestroy);

        TSharedPtr<FJsonObject> LocJson = MakeShareable(new FJsonObject);
        LocJson->SetNumberField(TEXT("x"), NiagaraActor->GetActorLocation().X);
        LocJson->SetNumberField(TEXT("y"), NiagaraActor->GetActorLocation().Y);
        LocJson->SetNumberField(TEXT("z"), NiagaraActor->GetActorLocation().Z);
        Result->SetObjectField(TEXT("location"), LocJson);

        TSharedPtr<FJsonObject> RotJson = MakeShareable(new FJsonObject);
        RotJson->SetNumberField(TEXT("pitch"), NiagaraActor->GetActorRotation().Pitch);
        RotJson->SetNumberField(TEXT("yaw"), NiagaraActor->GetActorRotation().Yaw);
        RotJson->SetNumberField(TEXT("roll"), NiagaraActor->GetActorRotation().Roll);
        Result->SetObjectField(TEXT("rotation"), RotJson);

        TSharedPtr<FJsonObject> ScaleJson = MakeShareable(new FJsonObject);
        ScaleJson->SetNumberField(TEXT("x"), NiagaraActor->GetActorScale3D().X);
        ScaleJson->SetNumberField(TEXT("y"), NiagaraActor->GetActorScale3D().Y);
        ScaleJson->SetNumberField(TEXT("z"), NiagaraActor->GetActorScale3D().Z);
        Result->SetObjectField(TEXT("scale"), ScaleJson);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSpawnNiagaraActorHandler)
