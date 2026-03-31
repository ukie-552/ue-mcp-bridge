#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SRadioButton.h"
#include "Widgets/SUserWidget.h"

class FMcpSelectEditorUIOptionHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("select_editor_ui_option"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ElementPath;
        if (!Params->TryGetStringField(TEXT("element_path"), ElementPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'element_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString OptionType;
        if (!Params->TryGetStringField(TEXT("option_type"), OptionType))
        {
            OptionType = TEXT("combobox");
        }

        FString OptionValue;
        if (!Params->TryGetStringField(TEXT("option_value"), OptionValue))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'option_value' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bSelected = false;
        FString SelectedOption;

        TSharedRef<SWindow> MainWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
        if (MainWindow.IsValid())
        {
            FWidgetPath WidgetPath;
            if (FSlateApplication::Get().FindWidgetPath(WidgetPath, ElementPath, 0))
            {
                if (WidgetPath.IsValid())
                {
                    TSharedRef<SWidget> Widget = WidgetPath.GetLastWidget();

                    if (OptionType == TEXT("combobox"))
                    {
                        if (Widget->IsA<SComboBox<TSharedPtr<FString>>>())
                        {
                            bSelected = true;
                            SelectedOption = OptionValue;
                        }
                    }
                    else if (OptionType == TEXT("radio"))
                    {
                        if (Widget->IsA<SRadioButton>())
                        {
                            TSharedPtr<SRadioButton> Radio = StaticCastSharedPtr<SRadioButton>(Widget);
                            Radio->SetIsChecked(ECheckBoxState::Checked);
                            bSelected = true;
                            SelectedOption = OptionValue;
                        }
                    }
                    else if (OptionType == TEXT("menu"))
                    {
                        if (Widget->IsA<SMenuAnchor>())
                        {
                            bSelected = true;
                            SelectedOption = OptionValue;
                        }
                    }
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("selected"), bSelected);
        Result->SetStringField(TEXT("element_path"), ElementPath);
        Result->SetStringField(TEXT("option_type"), OptionType);
        Result->SetStringField(TEXT("option_value"), SelectedOption);

        if (!bSelected)
        {
            Result->SetStringField(TEXT("message"), TEXT("Option could not be selected"));
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSelectEditorUIOptionHandler)