#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Editor.h"
#include "EngineUtils.h"

class FMcpRotateActorHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("rotate_actor"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        FRotator NewRotation = TargetActor->GetActorRotation();

        const TSharedPtr<FJsonObject>* RotationObj;
        if (Params->TryGetObjectField(TEXT("rotation"), RotationObj))
        {
            double Pitch = 0, Yaw = 0, Roll = 0;
            (*RotationObj)->TryGetNumberField(TEXT("pitch"), Pitch);
            (*RotationObj)->TryGetNumberField(TEXT("yaw"), Yaw);
            (*RotationObj)->TryGetNumberField(TEXT("roll"), Roll);
            NewRotation = FRotator(Pitch, Yaw, Roll);
        }

        bool bAdditive = false;
        Params->TryGetBoolField(TEXT("additive"), bAdditive);

        TargetActor->Modify();

        if (bAdditive)
        {
            TargetActor->AddActorLocalRotation(NewRotation);
        }
        else
        {
            TargetActor->SetActorRotation(NewRotation);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("rotated"), true);
        Result->SetStringField(TEXT("actor_id"), ActorId);

        FRotator FinalRotation = TargetActor->GetActorRotation();
        TSharedPtr<FJsonObject> RotJson = MakeShareable(new FJsonObject);
        RotJson->SetNumberField(TEXT("pitch"), FinalRotation.Pitch);
        RotJson->SetNumberField(TEXT("yaw"), FinalRotation.Yaw);
        RotJson->SetNumberField(TEXT("roll"), FinalRotation.Roll);
        Result->SetObjectField(TEXT("rotation"), RotJson);

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

REGISTER_MCP_COMMAND(FMcpRotateActorHandler)
