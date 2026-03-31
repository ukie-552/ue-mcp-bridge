#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "EngineUtils.h"

class FMcpSelectComponentInDetailsPanelHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("select_component_in_details_panel"); }

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

        UActorComponent* TargetComponent = nullptr;
        for (UActorComponent* Component : TargetActor->GetComponents())
        {
            if (Component && Component->GetName() == ComponentName)
            {
                TargetComponent = Component;
                break;
            }
        }

        if (!TargetComponent)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Component not found: %s"), *ComponentName),
                TEXT("COMPONENT_NOT_FOUND")
            );
        }

        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        TArray<IDetailsView*> DetailsViews = PropertyModule.GetPropertyViews();

        bool bSelected = false;
        for (IDetailsView* DetailsView : DetailsViews)
        {
            TArray<UObject*> Objects;
            Objects.Add(TargetComponent);
            DetailsView->SetObjects(Objects, true);
            bSelected = true;
            break;
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("selected"), bSelected);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetStringField(TEXT("component_path"), TargetComponent->GetPathName());

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

REGISTER_MCP_COMMAND(FMcpSelectComponentInDetailsPanelHandler)