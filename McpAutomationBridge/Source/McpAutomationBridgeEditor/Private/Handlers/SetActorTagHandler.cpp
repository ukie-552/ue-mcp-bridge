#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"

class FMcpSetActorTagHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_actor_tag"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString Tag;
        if (!Params->TryGetStringField(TEXT("tag"), Tag))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'tag' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bAdd = true;
        Params->TryGetBoolField(TEXT("add"), bAdd);

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        TargetActor->Modify();

        if (bAdd)
        {
            TargetActor->Tags.Add(*Tag);
        }
        else
        {
            TargetActor->Tags.Remove(*Tag);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("tag"), Tag);
        Result->SetBoolField(TEXT("added"), bAdd);

        TArray<TSharedPtr<FJsonValue>> TagsArray;
        for (const FName& ActorTag : TargetActor->Tags)
        {
            TagsArray.Add(MakeShareable(new FJsonValueString(ActorTag.ToString())));
        }
        Result->SetArrayField(TEXT("all_tags"), TagsArray);

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

REGISTER_MCP_COMMAND(FMcpSetActorTagHandler)
