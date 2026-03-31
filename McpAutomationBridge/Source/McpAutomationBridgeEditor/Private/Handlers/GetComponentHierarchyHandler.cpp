#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"
#include "EngineUtils.h"

class FMcpGetComponentHierarchyHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_component_hierarchy"); }

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

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("actor_name"), TargetActor->GetName());
        Result->SetStringField(TEXT("actor_class"), TargetActor->GetClass()->GetName());

        TSharedPtr<FJsonObject> RootObj = BuildComponentHierarchy(TargetActor->GetRootComponent());
        Result->SetObjectField(TEXT("root_component"), RootObj);

        TArray<TSharedPtr<FJsonValue>> AllComponents;
        CollectAllComponents(TargetActor->GetRootComponent(), AllComponents);
        Result->SetArrayField(TEXT("all_components"), AllComponents);
        Result->SetNumberField(TEXT("component_count"), AllComponents.Num());

        return FMcpCommandResult::Success(Result);
    }

private:
    TSharedPtr<FJsonObject> BuildComponentHierarchy(USceneComponent* Component, int32 Depth = 0)
    {
        TSharedPtr<FJsonObject> CompObj = MakeShareable(new FJsonObject);
        
        if (!Component)
        {
            return CompObj;
        }

        CompObj->SetStringField(TEXT("name"), Component->GetName());
        CompObj->SetStringField(TEXT("class"), Component->GetClass()->GetName());
        CompObj->SetStringField(TEXT("path"), Component->GetPathName());
        CompObj->SetNumberField(TEXT("depth"), Depth);

        FTransform Transform = Component->GetRelativeTransform();
        TSharedPtr<FJsonObject> LocationObj = MakeShareable(new FJsonObject);
        LocationObj->SetNumberField(TEXT("x"), Transform.GetLocation().X);
        LocationObj->SetNumberField(TEXT("y"), Transform.GetLocation().Y);
        LocationObj->SetNumberField(TEXT("z"), Transform.GetLocation().Z);
        CompObj->SetObjectField(TEXT("relative_location"), LocationObj);

        TSharedPtr<FJsonObject> RotationObj = MakeShareable(new FJsonObject);
        RotationObj->SetNumberField(TEXT("pitch"), Transform.GetRotation().Rotator().Pitch);
        RotationObj->SetNumberField(TEXT("yaw"), Transform.GetRotation().Rotator().Yaw);
        RotationObj->SetNumberField(TEXT("roll"), Transform.GetRotation().Rotator().Roll);
        CompObj->SetObjectField(TEXT("relative_rotation"), RotationObj);

        TSharedPtr<FJsonObject> ScaleObj = MakeShareable(new FJsonObject);
        ScaleObj->SetNumberField(TEXT("x"), Transform.GetScale3D().X);
        ScaleObj->SetNumberField(TEXT("y"), Transform.GetScale3D().Y);
        ScaleObj->SetNumberField(TEXT("z"), Transform.GetScale3D().Z);
        CompObj->SetObjectField(TEXT("relative_scale"), ScaleObj);

        TArray<TSharedPtr<FJsonValue>> ChildrenArray;
        const TArray<USceneComponent*>& Children = Component->GetAttachChildren();
        for (USceneComponent* Child : Children)
        {
            TSharedPtr<FJsonObject> ChildObj = BuildComponentHierarchy(Child, Depth + 1);
            ChildrenArray.Add(MakeShareable(new FJsonValueObject(ChildObj)));
        }
        CompObj->SetArrayField(TEXT("children"), ChildrenArray);
        CompObj->SetNumberField(TEXT("child_count"), Children.Num());

        return CompObj;
    }

    void CollectAllComponents(USceneComponent* Root, TArray<TSharedPtr<FJsonValue>>& OutArray)
    {
        if (!Root) return;

        for (UActorComponent* Component : Root->GetOwner()->GetComponents())
        {
            if (Component)
            {
                TSharedPtr<FJsonObject> CompInfo = MakeShareable(new FJsonObject);
                CompInfo->SetStringField(TEXT("name"), Component->GetName());
                CompInfo->SetStringField(TEXT("class"), Component->GetClass()->GetName());
                CompInfo->SetStringField(TEXT("path"), Component->GetPathName());
                CompInfo->SetBoolField(TEXT("is_scene_component"), Component->IsA<USceneComponent>());
                OutArray.Add(MakeShareable(new FJsonValueObject(CompInfo)));
            }
        }

        for (USceneComponent* Child : Root->GetAttachChildren())
        {
            CollectAllComponents(Child, OutArray);
        }
    }

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

REGISTER_MCP_COMMAND(FMcpGetComponentHierarchyHandler)