#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"
#include "EngineUtils.h"

class FMcpDuplicateComponentHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("duplicate_component"); }

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

        FString NewName;
        Params->TryGetStringField(TEXT("new_name"), NewName);

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        UActorComponent* SourceComponent = nullptr;
        for (UActorComponent* Component : TargetActor->GetComponents())
        {
            if (Component && Component->GetName() == ComponentName)
            {
                SourceComponent = Component;
                break;
            }
        }

        if (!SourceComponent)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Component not found: %s"), *ComponentName),
                TEXT("COMPONENT_NOT_FOUND")
            );
        }

        UActorComponent* NewComponent = DuplicateObject<UActorComponent>(SourceComponent, TargetActor);
        if (NewComponent)
        {
            if (!NewName.IsEmpty())
            {
                NewComponent->Rename(*NewName);
            }
            else
            {
                FString BaseName = NewComponent->GetName();
                FString UniqueName = BaseName;
                int32 Counter = 1;
                while (TargetActor->GetComponentByName(*UniqueName))
                {
                    UniqueName = FString::Printf(TEXT("%s_%d"), *BaseName, Counter++);
                }
                NewComponent->Rename(*UniqueName);
            }
            
            NewComponent->RegisterComponent();
            TargetActor->RerunConstructionScripts();
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("duplicated"), NewComponent != nullptr);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("source_component"), ComponentName);
        
        if (NewComponent)
        {
            Result->SetStringField(TEXT("new_component_name"), NewComponent->GetName());
            Result->SetStringField(TEXT("new_component_path"), NewComponent->GetPathName());
        }

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

REGISTER_MCP_COMMAND(FMcpDuplicateComponentHandler)