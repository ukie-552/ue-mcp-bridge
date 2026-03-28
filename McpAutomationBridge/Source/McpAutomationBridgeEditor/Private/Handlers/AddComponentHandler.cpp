#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

class FMcpAddComponentHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_component"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ComponentClass;
        if (!Params->TryGetStringField(TEXT("component_class"), ComponentClass))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'component_class' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ComponentName;
        Params->TryGetStringField(TEXT("component_name"), ComponentName);

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        UClass* CompClass = LoadClass<UActorComponent>(nullptr, *ComponentClass);
        if (!CompClass)
        {
            UObject* Asset = LoadObject<UObject>(nullptr, *ComponentClass);
            if (Asset)
            {
                CompClass = Cast<UClass>(Asset);
            }
        }

        if (!CompClass)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Component class not found: %s"), *ComponentClass),
                TEXT("CLASS_NOT_FOUND")
            );
        }

        UActorComponent* NewComponent = NewObject<UActorComponent>(TargetActor, CompClass, *ComponentName);
        if (!NewComponent)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to create component"), TEXT("COMPONENT_CREATION_FAILED"));
        }

        NewComponent->RegisterComponent();
        TargetActor->AddInstanceComponent(NewComponent);
        TargetActor->RerunConstructionScripts();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("added"), true);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("component_name"), NewComponent->GetName());
        Result->SetStringField(TEXT("component_class"), NewComponent->GetClass()->GetName());

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

REGISTER_MCP_COMMAND(FMcpAddComponentHandler)
