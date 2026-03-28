#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "Editor.h"

class FMcpGetCurrentLevelHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_current_level"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            return FMcpCommandResult::Failure(TEXT("No world available"), TEXT("NO_WORLD"));
        }

        FString LevelName = World->GetName();
        FString LevelPath = World->GetPathName();

        int32 ActorCount = 0;
        TArray<FString> ActorClasses;

        for (TActorIterator<AActor> It(World); It; ++It)
        {
            ActorCount++;
            FString ClassName = (*It)->GetClass()->GetName();
            if (!ActorClasses.Contains(ClassName))
            {
                ActorClasses.Add(ClassName);
            }
        }

        TArray<ULevel*> Levels = World->GetLevels();
        TArray<TSharedPtr<FJsonValue>> SubLevelsArray;
        for (ULevel* Level : Levels)
        {
            if (Level && Level != World->GetCurrentLevel())
            {
                TSharedPtr<FJsonObject> LevelInfo = MakeShareable(new FJsonObject);
                LevelInfo->SetStringField(TEXT("level_name"), Level->GetName());
                LevelInfo->SetNumberField(TEXT("actor_count"), Level->Actors.Num());
                SubLevelsArray.Add(MakeShareable(new FJsonValueObject(LevelInfo)));
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("level_name"), LevelName);
        Result->SetStringField(TEXT("level_path"), LevelPath);
        Result->SetNumberField(TEXT("actor_count"), ActorCount);
        Result->SetNumberField(TEXT("unique_class_count"), ActorClasses.Num());
        Result->SetArrayField(TEXT("actor_classes"), TArray<TSharedPtr<FJsonValue>>());
        Result->SetArrayField(TEXT("sub_levels"), SubLevelsArray);
        Result->SetNumberField(TEXT("sub_level_count"), SubLevelsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetCurrentLevelHandler)
