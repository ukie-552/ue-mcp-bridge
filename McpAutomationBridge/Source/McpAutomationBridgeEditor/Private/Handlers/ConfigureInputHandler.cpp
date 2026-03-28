#include "CoreMinimal.h"
#include "McpCommand.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"
#include "Engine/DataTable.h"
#include "Editor.h"

class FMcpConfigureInputHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("configure_input"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString InputType;
        if (!Params->TryGetStringField(TEXT("input_type"), InputType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'input_type' parameter"), TEXT("MISSING_PARAM"));
        }

        FString InputName;
        if (!Params->TryGetStringField(TEXT("input_name"), InputName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'input_name' parameter"), TEXT("MISSING_PARAM"));
        }

        UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
        if (!InputSettings)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to get input settings"), TEXT("SETTINGS_ERROR"));
        }

        if (InputType == TEXT("axis"))
        {
            const TArray<TSharedPtr<FJsonValue>>* AxisBindings;
            if (Params->TryGetArrayField(TEXT("bindings"), AxisBindings))
            {
                for (const TSharedPtr<FJsonValue>& Binding : *AxisBindings)
                {
                    TSharedPtr<FJsonObject> BindingObj = Binding->AsObject();
                    if (!BindingObj.IsValid()) continue;

                    FString KeyName;
                    BindingObj->TryGetStringField(TEXT("key"), KeyName);

                    float Scale = 1.0f;
                    BindingObj->TryGetNumberField(TEXT("scale"), Scale);

                    FKey Key(*KeyName);
                    if (!Key.IsValid()) continue;

                    FInputAxisKeyMapping NewMapping;
                    NewMapping.AxisName = *InputName;
                    NewMapping.Scale = Scale;
                    NewMapping.Key = Key;

                    InputSettings->AddAxisMapping(NewMapping);
                }
            }
        }
        else if (InputType == TEXT("action"))
        {
            const TArray<TSharedPtr<FJsonValue>>* ActionBindings;
            if (Params->TryGetArrayField(TEXT("bindings"), ActionBindings))
            {
                for (const TSharedPtr<FJsonValue>& Binding : *ActionBindings)
                {
                    TSharedPtr<FJsonObject> BindingObj = Binding->AsObject();
                    if (!BindingObj.IsValid()) continue;

                    FString KeyName;
                    BindingObj->TryGetStringField(TEXT("key"), KeyName);

                    FKey Key(*KeyName);
                    if (!Key.IsValid()) continue;

                    FInputActionKeyMapping NewMapping;
                    NewMapping.ActionName = *InputName;
                    NewMapping.Key = Key;

                    InputSettings->AddActionMapping(NewMapping);
                }
            }
        }
        else
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Unknown input type: %s"), *InputType),
                TEXT("INVALID_INPUT_TYPE")
            );
        }

        InputSettings->SaveKeyMappings();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("configured"), true);
        Result->SetStringField(TEXT("input_type"), InputType);
        Result->SetStringField(TEXT("input_name"), InputName);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpConfigureInputHandler)
