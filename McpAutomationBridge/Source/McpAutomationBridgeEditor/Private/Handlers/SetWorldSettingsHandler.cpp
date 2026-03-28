#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Editor.h"

class FMcpSetWorldSettingsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_world_settings"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            return FMcpCommandResult::Failure(TEXT("No world available"), TEXT("NO_WORLD"));
        }

        AWorldSettings* WorldSettings = World->GetWorldSettings();
        if (!WorldSettings)
        {
            return FMcpCommandResult::Failure(TEXT("No world settings found"), TEXT("NO_WORLD_SETTINGS"));
        }

        WorldSettings->Modify();

        bool bAnySet = false;

        FString GameModeClass;
        if (Params->TryGetStringField(TEXT("game_mode_class"), GameModeClass))
        {
            UClass* GameMode = LoadClass<AActor>(nullptr, *GameModeClass);
            if (GameMode)
            {
                WorldSettings->DefaultGameMode = GameMode;
                bAnySet = true;
            }
        }

        double TimeDilation = 1.0;
        if (Params->TryGetNumberField(TEXT("time_dilation"), TimeDilation))
        {
            WorldSettings->TimeDilation = TimeDilation;
            bAnySet = true;
        }

        double GravityZ = -980.0;
        if (Params->TryGetNumberField(TEXT("gravity_z"), GravityZ))
        {
            WorldSettings->WorldGravityZ = GravityZ;
            bAnySet = true;
        }

        bool bEnableWorldBoundsChecks = true;
        if (Params->TryGetBoolField(TEXT("enable_world_bounds_checks"), bEnableWorldBoundsChecks))
        {
            WorldSettings->bEnableWorldBoundsChecks = bEnableWorldBoundsChecks;
            bAnySet = true;
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), bAnySet);
        Result->SetStringField(TEXT("world_name"), World->GetName());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSetWorldSettingsHandler)
