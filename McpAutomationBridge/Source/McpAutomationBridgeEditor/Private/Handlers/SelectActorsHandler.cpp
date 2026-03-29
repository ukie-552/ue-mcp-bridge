#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#include "Selection.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "EngineUtils.h"

class FMcpSelectActorsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("select_actors"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        TArray<FString> ActorIds;
        const TArray<TSharedPtr<FJsonValue>>* ActorIdsArray;
        if (Params->TryGetArrayField(TEXT("actor_ids"), ActorIdsArray))
        {
            for (const auto& JsonValue : *ActorIdsArray)
            {
                ActorIds.Add(JsonValue->AsString());
            }
        }
        else
        {
            FString SingleActorId;
            if (Params->TryGetStringField(TEXT("actor_id"), SingleActorId))
            {
                ActorIds.Add(SingleActorId);
            }
            else
            {
                return FMcpCommandResult::Failure(TEXT("Missing 'actor_ids' or 'actor_id' parameter"), TEXT("MISSING_PARAM"));
            }
        }

        bool bAddToSelection = false;
        Params->TryGetBoolField(TEXT("add_to_selection"), bAddToSelection);

        bool bDeselectAll = true;
        Params->TryGetBoolField(TEXT("deselect_all_first"), bDeselectAll);

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            return FMcpCommandResult::Failure(TEXT("No editor world found"), TEXT("NO_WORLD"));
        }

        if (bDeselectAll && !bAddToSelection)
        {
            GEditor->SelectNone(false, true, false);
        }

        TArray<AActor*> SelectedActors;
        TArray<FString> NotFoundIds;

        for (const FString& ActorId : ActorIds)
        {
            AActor* Actor = FindActorById(ActorId);
            if (Actor)
            {
                GEditor->SelectActor(Actor, true, true, true, false);
                SelectedActors.Add(Actor);
            }
            else
            {
                NotFoundIds.Add(ActorId);
            }
        }

        GEditor->NoteSelectionChange();

        TArray<TSharedPtr<FJsonValue>> SelectedArray;
        for (AActor* Actor : SelectedActors)
        {
            TSharedPtr<FJsonObject> ActorInfo = MakeShareable(new FJsonObject);
            ActorInfo->SetStringField(TEXT("id"), Actor->GetActorGuid().ToString());
            ActorInfo->SetStringField(TEXT("name"), Actor->GetName());
            ActorInfo->SetStringField(TEXT("class"), Actor->GetClass()->GetName());
            SelectedArray.Add(MakeShareable(new FJsonValueObject(ActorInfo)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("selected_actors"), SelectedArray);
        Result->SetNumberField(TEXT("selected_count"), SelectedActors.Num());
        Result->SetBoolField(TEXT("success"), true);

        if (NotFoundIds.Num() > 0)
        {
            TArray<TSharedPtr<FJsonValue>> NotFoundArray;
            for (const FString& Id : NotFoundIds)
            {
                NotFoundArray.Add(MakeShareable(new FJsonValueString(Id)));
            }
            Result->SetArrayField(TEXT("not_found_ids"), NotFoundArray);
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

REGISTER_MCP_COMMAND(FMcpSelectActorsHandler)
