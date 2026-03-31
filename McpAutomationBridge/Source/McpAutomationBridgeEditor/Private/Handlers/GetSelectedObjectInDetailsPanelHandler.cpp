#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"

class FMcpGetSelectedObjectInDetailsPanelHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_selected_object_in_details_panel"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);

        TArray<TWeakObjectPtr<UObject>> SelectedObjects;
        TArray<TWeakObjectPtr<AActor>> SelectedActors;

        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        TArray<IDetailsView*> DetailsViews = PropertyModule.GetPropertyViews();

        if (DetailsViews.Num() > 0)
        {
            IDetailsView* DetailsView = DetailsViews[0];
            SelectedObjects = DetailsView->GetSelectedObjects();
            SelectedActors = DetailsView->GetSelectedActors();
        }

        TArray<TSharedPtr<FJsonValue>> ObjectsArray;
        for (const TWeakObjectPtr<UObject>& Obj : SelectedObjects)
        {
            if (Obj.IsValid())
            {
                TSharedPtr<FJsonObject> ObjInfo = MakeShareable(new FJsonObject);
                ObjInfo->SetStringField(TEXT("object_path"), Obj->GetPathName());
                ObjInfo->SetStringField(TEXT("object_class"), Obj->GetClass()->GetName());
                ObjInfo->SetStringField(TEXT("object_name"), Obj->GetName());
                ObjectsArray.Add(MakeShareable(new FJsonValueObject(ObjInfo)));
            }
        }

        TArray<TSharedPtr<FJsonValue>> ActorsArray;
        for (const TWeakObjectPtr<AActor>& Actor : SelectedActors)
        {
            if (Actor.IsValid())
            {
                TSharedPtr<FJsonObject> ActorInfo = MakeShareable(new FJsonObject);
                ActorInfo->SetStringField(TEXT("actor_id"), Actor->GetActorGuid().ToString());
                ActorInfo->SetStringField(TEXT("actor_name"), Actor->GetName());
                ActorInfo->SetStringField(TEXT("actor_class"), Actor->GetClass()->GetName());
                ActorInfo->SetStringField(TEXT("actor_path"), Actor->GetPathName());
                ActorsArray.Add(MakeShareable(new FJsonValueObject(ActorInfo)));
            }
        }

        Result->SetArrayField(TEXT("selected_objects"), ObjectsArray);
        Result->SetArrayField(TEXT("selected_actors"), ActorsArray);
        Result->SetNumberField(TEXT("object_count"), SelectedObjects.Num());
        Result->SetNumberField(TEXT("actor_count"), SelectedActors.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetSelectedObjectInDetailsPanelHandler)