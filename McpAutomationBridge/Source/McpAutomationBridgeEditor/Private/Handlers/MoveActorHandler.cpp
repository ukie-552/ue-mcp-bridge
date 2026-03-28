#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "EditorLevelLibrary.h"

class FMcpMoveActorHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("move_actor"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        const TSharedPtr<FJsonObject>* LocationObj;
        if (!Params->TryGetObjectField(TEXT("location"), LocationObj))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'location' parameter"), TEXT("MISSING_PARAM"));
        }

        double X = 0, Y = 0, Z = 0;
        (*LocationObj)->TryGetNumberField(TEXT("x"), X);
        (*LocationObj)->TryGetNumberField(TEXT("y"), Y);
        (*LocationObj)->TryGetNumberField(TEXT("z"), Z);
        FVector NewLocation(X, Y, Z);

        bool bRelative = false;
        Params->TryGetBoolField(TEXT("relative"), bRelative);

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        TargetActor->Modify();

        if (bRelative)
        {
            TargetActor->AddActorWorldOffset(NewLocation);
        }
        else
        {
            TargetActor->SetActorLocation(NewLocation);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("actor_id"), ActorId);

        FVector FinalLocation = TargetActor->GetActorLocation();
        TSharedPtr<FJsonObject> LocJson = MakeShareable(new FJsonObject);
        LocJson->SetNumberField(TEXT("x"), FinalLocation.X);
        LocJson->SetNumberField(TEXT("y"), FinalLocation.Y);
        LocJson->SetNumberField(TEXT("z"), FinalLocation.Z);
        Result->SetObjectField(TEXT("location"), LocJson);

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

REGISTER_MCP_COMMAND(FMcpMoveActorHandler)
