#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Editor.h"
#include "EngineUtils.h"

class FMcpGetActorInfoHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_actor_info"); }

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
        Result->SetStringField(TEXT("actor_name"), TargetActor->GetActorLabel());
        Result->SetStringField(TEXT("actor_class"), TargetActor->GetClass()->GetName());
        Result->SetStringField(TEXT("actor_path"), TargetActor->GetPathName());

        FVector Location = TargetActor->GetActorLocation();
        TSharedPtr<FJsonObject> LocJson = MakeShareable(new FJsonObject);
        LocJson->SetNumberField(TEXT("x"), Location.X);
        LocJson->SetNumberField(TEXT("y"), Location.Y);
        LocJson->SetNumberField(TEXT("z"), Location.Z);
        Result->SetObjectField(TEXT("location"), LocJson);

        FRotator Rotation = TargetActor->GetActorRotation();
        TSharedPtr<FJsonObject> RotJson = MakeShareable(new FJsonObject);
        RotJson->SetNumberField(TEXT("pitch"), Rotation.Pitch);
        RotJson->SetNumberField(TEXT("yaw"), Rotation.Yaw);
        RotJson->SetNumberField(TEXT("roll"), Rotation.Roll);
        Result->SetObjectField(TEXT("rotation"), RotJson);

        FVector Scale = TargetActor->GetActorScale3D();
        TSharedPtr<FJsonObject> ScaleJson = MakeShareable(new FJsonObject);
        ScaleJson->SetNumberField(TEXT("x"), Scale.X);
        ScaleJson->SetNumberField(TEXT("y"), Scale.Y);
        ScaleJson->SetNumberField(TEXT("z"), Scale.Z);
        Result->SetObjectField(TEXT("scale"), ScaleJson);

        Result->SetBoolField(TEXT("is_hidden"), TargetActor->IsHidden());
        Result->SetBoolField(TEXT("is_temporarily_hidden"), TargetActor->IsTemporarilyHiddenInEditor());

        TArray<TSharedPtr<FJsonValue>> TagsArray;
        for (const FName& Tag : TargetActor->Tags)
        {
            TagsArray.Add(MakeShareable(new FJsonValueString(Tag.ToString())));
        }
        Result->SetArrayField(TEXT("tags"), TagsArray);

        TArray<TSharedPtr<FJsonValue>> ComponentsArray;
        for (UActorComponent* Component : TargetActor->GetComponents())
        {
            if (Component)
            {
                TSharedPtr<FJsonObject> CompInfo = MakeShareable(new FJsonObject);
                CompInfo->SetStringField(TEXT("component_name"), Component->GetName());
                CompInfo->SetStringField(TEXT("component_class"), Component->GetClass()->GetName());
                ComponentsArray.Add(MakeShareable(new FJsonValueObject(CompInfo)));
            }
        }
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

REGISTER_MCP_COMMAND(FMcpGetActorInfoHandler)
