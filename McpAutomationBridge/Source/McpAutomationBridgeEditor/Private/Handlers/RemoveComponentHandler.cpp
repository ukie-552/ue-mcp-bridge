#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Editor.h"
#include "EngineUtils.h"

class FMcpRemoveComponentHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("remove_component"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ComponentName;
        if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'component_name' parameter"), TEXT("MISSING_PARAM"));
        }

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        UActorComponent* ComponentToRemove = nullptr;
        
        for (UActorComponent* Component : TargetActor->GetComponents())
        {
            if (Component && Component->GetName() == ComponentName)
            {
                ComponentToRemove = Component;
                break;
            }
        }

        if (!ComponentToRemove)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Component not found: %s"), *ComponentName),
                TEXT("COMPONENT_NOT_FOUND")
            );
        }

        FString RemovedComponentClass = ComponentToRemove->GetClass()->GetName();
        
        TargetActor->RemoveInstanceComponent(ComponentToRemove);
        ComponentToRemove->DestroyComponent();
        TargetActor->RerunConstructionScripts();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("removed"), true);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetStringField(TEXT("component_class"), RemovedComponentClass);

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

REGISTER_MCP_COMMAND(FMcpRemoveComponentHandler)
