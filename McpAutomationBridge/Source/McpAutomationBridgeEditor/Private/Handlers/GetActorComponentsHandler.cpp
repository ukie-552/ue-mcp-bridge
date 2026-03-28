#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"

class FMcpGetActorComponentsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_actor_components"); }

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

        bool bIncludeProperties = false;
        Params->TryGetBoolField(TEXT("include_properties"), bIncludeProperties);

        TArray<TSharedPtr<FJsonValue>> ComponentsArray;

        for (UActorComponent* Component : TargetActor->GetComponents())
        {
            if (Component)
            {
                TSharedPtr<FJsonObject> CompInfo = MakeShareable(new FJsonObject);
                CompInfo->SetStringField(TEXT("name"), Component->GetName());
                CompInfo->SetStringField(TEXT("class"), Component->GetClass()->GetName());
                CompInfo->SetStringField(TEXT("path"), Component->GetPathName());

                USceneComponent* SceneComp = Cast<USceneComponent>(Component);
                if (SceneComp)
                {
                    CompInfo->SetBoolField(TEXT("is_scene_component"), true);
                    
                    TSharedPtr<FJsonObject> TransformInfo = MakeShareable(new FJsonObject);
                    
                    FVector Location = SceneComp->GetRelativeLocation();
                    TSharedPtr<FJsonObject> LocObj = MakeShareable(new FJsonObject);
                    LocObj->SetNumberField(TEXT("x"), Location.X);
                    LocObj->SetNumberField(TEXT("y"), Location.Y);
                    LocObj->SetNumberField(TEXT("z"), Location.Z);
                    TransformInfo->SetObjectField(TEXT("location"), LocObj);

                    FRotator Rotation = SceneComp->GetRelativeRotation();
                    TSharedPtr<FJsonObject> RotObj = MakeShareable(new FJsonObject);
                    RotObj->SetNumberField(TEXT("pitch"), Rotation.Pitch);
                    RotObj->SetNumberField(TEXT("yaw"), Rotation.Yaw);
                    RotObj->SetNumberField(TEXT("roll"), Rotation.Roll);
                    TransformInfo->SetObjectField(TEXT("rotation"), RotObj);

                    FVector Scale = SceneComp->GetRelativeScale3D();
                    TSharedPtr<FJsonObject> ScaleObj = MakeShareable(new FJsonObject);
                    ScaleObj->SetNumberField(TEXT("x"), Scale.X);
                    ScaleObj->SetNumberField(TEXT("y"), Scale.Y);
                    ScaleObj->SetNumberField(TEXT("z"), Scale.Z);
                    TransformInfo->SetObjectField(TEXT("scale"), ScaleObj);

                    CompInfo->SetObjectField(TEXT("transform"), TransformInfo);
                }
                else
                {
                    CompInfo->SetBoolField(TEXT("is_scene_component"), false);
                }

                if (bIncludeProperties)
                {
                    TArray<TSharedPtr<FJsonValue>> PropertiesArray;
                    for (TFieldIterator<FProperty> PropIt(Component->GetClass()); PropIt; ++PropIt)
                    {
                        FProperty* Property = *PropIt;
                        if (Property->HasAnyPropertyFlags(CPF_Edit) && !Property->HasAnyPropertyFlags(CPF_Transient))
                        {
                            TSharedPtr<FJsonObject> PropInfo = MakeShareable(new FJsonObject);
                            PropInfo->SetStringField(TEXT("name"), Property->GetName());
                            PropInfo->SetStringField(TEXT("type"), Property->GetClass()->GetName());
                            PropertiesArray.Add(MakeShareable(new FJsonValueObject(PropInfo)));
                        }
                    }
                    CompInfo->SetArrayField(TEXT("properties"), PropertiesArray);
                }

                ComponentsArray.Add(MakeShareable(new FJsonValueObject(CompInfo)));
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("actor_name"), TargetActor->GetName());
        Result->SetArrayField(TEXT("components"), ComponentsArray);
        Result->SetNumberField(TEXT("component_count"), ComponentsArray.Num());

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

REGISTER_MCP_COMMAND(FMcpGetActorComponentsHandler)
