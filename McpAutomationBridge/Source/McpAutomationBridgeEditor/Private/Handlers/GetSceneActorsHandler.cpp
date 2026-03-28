#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "EditorLevelLibrary.h"

class FMcpGetSceneActorsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_scene_actors"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            return FMcpCommandResult::Failure(TEXT("No editor world available"), TEXT("NO_WORLD"));
        }

        FString ClassNameFilter;
        FString TagFilter;
        FString NameContains;
        bool bIncludeHidden = false;

        const TSharedPtr<FJsonObject>* FilterObj;
        if (Params->TryGetObjectField(TEXT("filter"), FilterObj))
        {
            (*FilterObj)->TryGetStringField(TEXT("class_name"), ClassNameFilter);
            (*FilterObj)->TryGetStringField(TEXT("tag"), TagFilter);
            (*FilterObj)->TryGetStringField(TEXT("name_contains"), NameContains);
        }
        Params->TryGetBoolField(TEXT("include_hidden"), bIncludeHidden);

        TArray<TSharedPtr<FJsonValue>> ActorsArray;

        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor) continue;

            if (!bIncludeHidden && Actor->IsHiddenEd()) continue;

            if (!ClassNameFilter.IsEmpty())
            {
                if (!Actor->GetClass()->GetName().Contains(ClassNameFilter) &&
                    !Actor->GetClass()->GetSuperClass()->GetName().Contains(ClassNameFilter))
                {
                    continue;
                }
            }

            if (!TagFilter.IsEmpty())
            {
                if (!Actor->ActorHasTag(*TagFilter))
                {
                    continue;
                }
            }

            if (!NameContains.IsEmpty())
            {
                if (!Actor->GetActorLabel().Contains(NameContains))
                {
                    continue;
                }
            }

            TSharedPtr<FJsonObject> ActorJson = MakeShareable(new FJsonObject);
            ActorJson->SetStringField(TEXT("actor_id"), Actor->GetActorGuid().ToString());
            ActorJson->SetStringField(TEXT("actor_name"), Actor->GetActorLabel());
            ActorJson->SetStringField(TEXT("class_name"), Actor->GetClass()->GetName());
            ActorJson->SetStringField(TEXT("path"), Actor->GetPathName());

            FVector Location = Actor->GetActorLocation();
            TSharedPtr<FJsonObject> LocJson = MakeShareable(new FJsonObject);
            LocJson->SetNumberField(TEXT("x"), Location.X);
            LocJson->SetNumberField(TEXT("y"), Location.Y);
            LocJson->SetNumberField(TEXT("z"), Location.Z);
            ActorJson->SetObjectField(TEXT("location"), LocJson);

            FRotator Rotation = Actor->GetActorRotation();
            TSharedPtr<FJsonObject> RotJson = MakeShareable(new FJsonObject);
            RotJson->SetNumberField(TEXT("pitch"), Rotation.Pitch);
            RotJson->SetNumberField(TEXT("yaw"), Rotation.Yaw);
            RotJson->SetNumberField(TEXT("roll"), Rotation.Roll);
            ActorJson->SetObjectField(TEXT("rotation"), RotJson);

            FVector Scale = Actor->GetActorScale3D();
            TSharedPtr<FJsonObject> ScaleJson = MakeShareable(new FJsonObject);
            ScaleJson->SetNumberField(TEXT("x"), Scale.X);
            ScaleJson->SetNumberField(TEXT("y"), Scale.Y);
            ScaleJson->SetNumberField(TEXT("z"), Scale.Z);
            ActorJson->SetObjectField(TEXT("scale"), ScaleJson);

            TArray<FString> Tags;
            for (const FName& Tag : Actor->Tags)
            {
                Tags.Add(Tag.ToString());
            }
            
            TArray<TSharedPtr<FJsonValue>> TagsArray;
            for (const FString& Tag : Tags)
            {
                TagsArray.Add(MakeShareable(new FJsonValueString(Tag)));
            }
            ActorJson->SetArrayField(TEXT("tags"), TagsArray);

            ActorsArray.Add(MakeShareable(new FJsonValueObject(ActorJson)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("actors"), ActorsArray);
        Result->SetNumberField(TEXT("count"), ActorsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetSceneActorsHandler)
