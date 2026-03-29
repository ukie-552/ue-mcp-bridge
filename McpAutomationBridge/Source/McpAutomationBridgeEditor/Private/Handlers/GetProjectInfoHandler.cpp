#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Misc/MessageDialog.h"
#include "Misc/EngineVersion.h"
#include "EngineUtils.h"

class FMcpGetProjectInfoHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_project_info"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);

        FString ProjectName = FApp::GetProjectName();
        Result->SetStringField(TEXT("project_name"), ProjectName);

        FString ProjectDir = FPaths::ProjectDir();
        Result->SetStringField(TEXT("project_dir"), ProjectDir);

        FString EngineDir = FPaths::EngineDir();
        Result->SetStringField(TEXT("engine_dir"), EngineDir);

        FString EngineVersion = FString::Printf(TEXT("%d.%d.%d"), 
            FEngineVersion::Current().GetMajor(), 
            FEngineVersion::Current().GetMinor(), 
            FEngineVersion::Current().GetPatch());
        Result->SetStringField(TEXT("engine_version"), EngineVersion);

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (World)
        {
            Result->SetStringField(TEXT("current_level"), World->GetName());

            int32 ActorCount = 0;
            for (TActorIterator<AActor> It(World); It; ++It)
            {
                ActorCount++;
            }
            Result->SetNumberField(TEXT("actor_count"), ActorCount);
        }

        Result->SetBoolField(TEXT("is_playing"), GEditor->IsPlayingSessionInEditor());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetProjectInfoHandler)
