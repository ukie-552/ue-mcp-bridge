#include "CoreMinimal.h"
#include "McpCommand.h"
#include "LevelSequence.h"
#include "MovieScene.h"
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "Sections/MovieSceneCameraCutSection.h"
#include "GameFramework/Actor.h"
#include "CineCameraActor.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"

class FMcpCreateCineCameraHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_cine_camera"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString CameraName;
        Params->TryGetStringField(TEXT("camera_name"), CameraName);

        if (CameraName.IsEmpty())
        {
            CameraName = TEXT("CineCamera");
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

        bool bLookAtEnabled = false;
        Params->TryGetBoolField(TEXT("look_at_enabled"), bLookAtEnabled);

        FString LookAtTarget;
        Params->TryGetStringField(TEXT("look_at_target"), LookAtTarget);

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            return FMcpCommandResult::Failure(TEXT("No editor world available"), TEXT("NO_WORLD"));
        }

        UClass* CameraClass = ACineCameraActor::StaticClass();
        if (!CameraClass)
        {
            CameraClass = ACameraActor::StaticClass();
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* NewCamera = World->SpawnActor<AActor>(CameraClass, Location, Rotation, SpawnParams);
        if (!NewCamera)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to create camera"), TEXT("CAMERA_SPAWN_FAILED"));
        }

        NewCamera->SetActorLabel(*CameraName);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("camera_name"), NewCamera->GetActorLabel());
        Result->SetStringField(TEXT("camera_class"), NewCamera->GetClass()->GetName());

        TSharedPtr<FJsonObject> LocJson = MakeShareable(new FJsonObject);
        LocJson->SetNumberField(TEXT("x"), NewCamera->GetActorLocation().X);
        LocJson->SetNumberField(TEXT("y"), NewCamera->GetActorLocation().Y);
        LocJson->SetNumberField(TEXT("z"), NewCamera->GetActorLocation().Z);
        Result->SetObjectField(TEXT("location"), LocJson);

        TSharedPtr<FJsonObject> RotJson = MakeShareable(new FJsonObject);
        RotJson->SetNumberField(TEXT("pitch"), NewCamera->GetActorRotation().Pitch);
        RotJson->SetNumberField(TEXT("yaw"), NewCamera->GetActorRotation().Yaw);
        RotJson->SetNumberField(TEXT("roll"), NewCamera->GetActorRotation().Roll);
        Result->SetObjectField(TEXT("rotation"), RotJson);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateCineCameraHandler)
