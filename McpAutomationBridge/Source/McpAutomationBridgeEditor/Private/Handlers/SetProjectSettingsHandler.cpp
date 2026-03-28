#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "ISettingsModule.h"
#include "ISettingsContainer.h"
#include "ISettingsSection.h"
#include "GameFramework/GameUserSettings.h"

class FMcpSetProjectSettingsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_project_settings"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Category;
        if (!Params->TryGetStringField(TEXT("category"), Category))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'category' parameter"), TEXT("MISSING_PARAM"));
        }

        FString SettingName;
        if (!Params->TryGetStringField(TEXT("setting_name"), SettingName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'setting_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString SettingValue;
        if (!Params->TryGetStringField(TEXT("setting_value"), SettingValue))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'setting_value' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bSet = false;

        if (Category == TEXT("Game") || Category == TEXT("GameUserSettings"))
        {
            UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
            if (UserSettings)
            {
                UserSettings->Modify();

                if (SettingName == TEXT("ScreenResolution"))
                {
                    FString WidthStr, HeightStr;
                    if (SettingValue.Split(TEXT("x"), &WidthStr, &HeightStr))
                    {
                        UserSettings->SetScreenResolution(FIntPoint(FCString::Atoi(*WidthStr), FCString::Atoi(*HeightStr)));
                        bSet = true;
                    }
                }
                else if (SettingName == TEXT("FullscreenMode"))
                {
                    int32 Mode = FCString::Atoi(*SettingValue);
                    UserSettings->SetFullscreenMode(static_cast<EWindowMode::Type>(Mode));
                    bSet = true;
                }
                else if (SettingName == TEXT("VSync"))
                {
                    UserSettings->SetVSyncEnabled(SettingValue == TEXT("true") || SettingValue == TEXT("1"));
                    bSet = true;
                }
                else if (SettingName == TEXT("ResolutionScale"))
                {
                    UserSettings->SetResolutionScaleValue(FCString::Atof(*SettingValue));
                    bSet = true;
                }

                if (bSet)
                {
                    UserSettings->ApplySettings(false);
                }
            }
        }
        else if (Category == TEXT("Engine") || Category == TEXT("Rendering"))
        {
            if (GEngine)
            {
                GEngine->Modify();

                if (SettingName == TEXT("bUseFixedFrameRate"))
                {
                    GEngine->bUseFixedFrameRate = (SettingValue == TEXT("true") || SettingValue == TEXT("1"));
                    bSet = true;
                }
                else if (SettingName == TEXT("FixedFrameRate"))
                {
                    GEngine->FixedFrameRate = FCString::Atof(*SettingValue);
                    bSet = true;
                }
            }
        }

        if (!bSet)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to set project setting: %s.%s"), *Category, *SettingName),
                TEXT("SETTING_NOT_FOUND")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), true);
        Result->SetStringField(TEXT("category"), Category);
        Result->SetStringField(TEXT("setting_name"), SettingName);
        Result->SetStringField(TEXT("setting_value"), SettingValue);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSetProjectSettingsHandler)
