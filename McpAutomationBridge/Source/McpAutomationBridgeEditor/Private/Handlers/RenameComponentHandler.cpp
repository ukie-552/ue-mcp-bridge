#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"

class FMcpRenameComponentHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("rename_component"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString OldName;
        if (!Params->TryGetStringField(TEXT("old_name"), OldName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'old_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NewName;
        if (!Params->TryGetStringField(TEXT("new_name"), NewName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'new_name' parameter"), TEXT("MISSING_PARAM"));
        }

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        USceneComponent* TargetComponent = nullptr;
        for (USceneComponent* Component : TargetActor->GetRootComponent()->GetAttachChildren())
        {
            if (Component && Component->GetName() == OldName)
            {
                TargetComponent = Component;
                break;
            }
        }

        for (UActorComponent* Component : TargetActor->GetComponents())
        {
            if (Component && Component->GetName() == OldName)
            {
                if (USceneComponent* SceneComp = Cast<USceneComponent>(Component))
                {
                    TargetComponent = SceneComp;
                    break;
                }
            }
        }

        if (!TargetComponent)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Component not found: %s"), *OldName),
                TEXT("COMPONENT_NOT_FOUND")
            );
        }

        TargetComponent->Rename(*NewName);
        TargetActor->RerunConstructionScripts();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("renamed"), true);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("old_name"), OldName);
        Result->SetStringField(TEXT("new_name"), NewName);

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

REGISTER_MCP_COMMAND(FMcpRenameComponentHandler)