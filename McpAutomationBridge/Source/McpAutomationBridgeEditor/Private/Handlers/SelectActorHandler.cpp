#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Selection.h"

class FMcpSelectActorHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("select_actor"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bAddToSelection = false;
        Params->TryGetBoolField(TEXT("add_to_selection"), bAddToSelection);

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        USelection* Selection = GEditor->GetSelectedActors();
        if (!Selection)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to get selection"), TEXT("SELECTION_ERROR"));
        }

        if (!bAddToSelection)
        {
            Selection->DeselectAll();
        }

        Selection->Select(TargetActor);
        GEditor->NoteSelectionChange();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("selected"), true);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("actor_name"), TargetActor->GetActorLabel());

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

REGISTER_MCP_COMMAND(FMcpSelectActorHandler)
