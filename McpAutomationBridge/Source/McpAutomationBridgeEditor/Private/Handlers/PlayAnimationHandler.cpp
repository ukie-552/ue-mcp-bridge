#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Editor.h"
#include "EngineUtils.h"

class FMcpPlayAnimationHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("play_animation"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString AnimationPath;
        if (!Params->TryGetStringField(TEXT("animation_path"), AnimationPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'animation_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bIsMontage = false;
        Params->TryGetBoolField(TEXT("is_montage"), bIsMontage);

        float PlayRate = 1.0f;
        Params->TryGetNumberField(TEXT("play_rate"), PlayRate);

        bool bLooping = false;
        Params->TryGetBoolField(TEXT("looping"), bLooping);

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        USkeletalMeshComponent* SkeletalMesh = TargetActor->FindComponentByClass<USkeletalMeshComponent>();
        if (!SkeletalMesh)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor has no SkeletalMeshComponent: %s"), *TargetActor->GetActorLabel()),
                TEXT("NO_SKELETAL_MESH")
            );
        }

        UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
        if (!AnimInstance)
        {
            return FMcpCommandResult::Failure(TEXT("No AnimInstance found on SkeletalMesh"), TEXT("NO_ANIM_INSTANCE"));
        }

        bool bPlaying = false;

        if (bIsMontage)
        {
            UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr, *AnimationPath);
            if (!Montage)
            {
                return FMcpCommandResult::Failure(
                    FString::Printf(TEXT("AnimMontage not found: %s"), *AnimationPath),
                    TEXT("ANIMATION_NOT_FOUND")
                );
            }

            AnimInstance->Montage_Play(Montage, PlayRate);
            bPlaying = true;
        }
        else
        {
            UAnimSequence* AnimSequence = LoadObject<UAnimSequence>(nullptr, *AnimationPath);
            if (!AnimSequence)
            {
                return FMcpCommandResult::Failure(
                    FString::Printf(TEXT("AnimSequence not found: %s"), *AnimationPath),
                    TEXT("ANIMATION_NOT_FOUND")
                );
            }

            int32 LoopCount = bLooping ? -1 : 1;
            AnimInstance->PlaySlotAnimationAsDynamicMontage(AnimSequence, TEXT("DefaultGroup"), 0.25f, 0.25f, PlayRate, LoopCount);
            bPlaying = true;
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("playing"), bPlaying);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("animation_path"), AnimationPath);
        Result->SetNumberField(TEXT("play_rate"), PlayRate);
        Result->SetBoolField(TEXT("is_montage"), bIsMontage);

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

REGISTER_MCP_COMMAND(FMcpPlayAnimationHandler)
