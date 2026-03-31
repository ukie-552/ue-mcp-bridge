#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SUserWidget.h"

class FMcpSetEditorUIElementValueHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_editor_ui_element_value"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ElementPath;
        if (!Params->TryGetStringField(TEXT("element_path"), ElementPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'element_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ValueType;
        if (!Params->TryGetStringField(TEXT("value_type"), ValueType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'value_type' parameter (text, number, bool, selection)"), TEXT("MISSING_PARAM"));
        }

        bool bSet = false;
        FString SetValue;

        TSharedRef<SWindow> MainWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
        if (MainWindow.IsValid())
        {
            FWidgetPath WidgetPath;
            if (FSlateApplication::Get().FindWidgetPath(WidgetPath, ElementPath, 0))
            {
                if (WidgetPath.IsValid())
                {
                    TSharedRef<SWidget> Widget = WidgetPath.GetLastWidget();

                    if (ValueType == TEXT("text"))
                    {
                        FString TextValue;
                        if (Params->TryGetStringField(TEXT("value"), TextValue))
                        {
                            if (Widget->IsA<SEditableText>())
                            {
                                TSharedPtr<SEditableText> EditableText = StaticCastSharedPtr<SEditableText>(Widget);
                                EditableText->SetText(FText::FromString(TextValue));
                                bSet = true;
                                SetValue = TextValue;
                            }
                        }
                    }
                    else if (ValueType == TEXT("number"))
                    {
                        double NumValue = 0;
                        if (Params->TryGetNumberField(TEXT("value"), NumValue))
                        {
                            bSet = true;
                            SetValue = FString::SanitizeFloat(NumValue);
                        }
                    }
                    else if (ValueType == TEXT("bool"))
                    {
                        bool BoolValue = false;
                        if (Params->TryGetBoolField(TEXT("value"), BoolValue))
                        {
                            bSet = true;
                            SetValue = BoolValue ? TEXT("true") : TEXT("false");
                        }
                    }
                    else if (ValueType == TEXT("selection"))
                    {
                        int32 IndexValue = 0;
                        if (Params->TryGetNumberField(TEXT("value"), IndexValue))
                        {
                            bSet = true;
                            SetValue = FString::FromInt(IndexValue);
                        }
                    }
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), bSet);
        Result->SetStringField(TEXT("element_path"), ElementPath);
        Result->SetStringField(TEXT("value_type"), ValueType);
        Result->SetStringField(TEXT("value"), SetValue);

        if (!bSet)
        {
            Result->SetStringField(TEXT("message"), TEXT("Element not found or value could not be set"));
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSetEditorUIElementValueHandler)