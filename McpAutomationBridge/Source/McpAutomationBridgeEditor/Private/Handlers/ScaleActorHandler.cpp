#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Editor.h"
#include "EngineUtils.h"

class FMcpScaleActorHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("scale_actor"); }

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

        FVector NewScale = TargetActor->GetActorScale3D();

        const TSharedPtr<FJsonObject>* ScaleObj;
        if (Params->TryGetObjectField(TEXT("scale"), ScaleObj))
        {
            double X = 1.0, Y = 1.0, Z = 1.0;
            (*ScaleObj)->TryGetNumberField(TEXT("x"), X);
            (*ScaleObj)->TryGetNumberField(TEXT("y"), Y);
            (*ScaleObj)->TryGetNumberField(TEXT("z"), Z);
            NewScale = FVector(X, Y, Z);
        }

        bool bUniform = false;
        Params->TryGetBoolField(TEXT("uniform"), bUniform);

        if (bUniform)
        {
            double UniformScale = 1.0;
            Params->TryGetNumberField(TEXT("uniform_scale"), UniformScale);
            NewScale = FVector(UniformScale);
        }

        TargetActor->Modify();
        TargetActor->SetActorScale3D(NewScale);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("scaled"), true);
        Result->SetStringField(TEXT("actor_id"), ActorId);

        FVector FinalScale = TargetActor->GetActorScale3D();
        TSharedPtr<FJsonObject> ScaleJson = MakeShareable(new FJsonObject);
        ScaleJson->SetNumberField(TEXT("x"), FinalScale.X);
        ScaleJson->SetNumberField(TEXT("y"), FinalScale.Y);
        ScaleJson->SetNumberField(TEXT("z"), FinalScale.Z);
        Result->SetObjectField(TEXT("scale"), ScaleJson);

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

REGISTER_MCP_COMMAND(FMcpScaleActorHandler)
