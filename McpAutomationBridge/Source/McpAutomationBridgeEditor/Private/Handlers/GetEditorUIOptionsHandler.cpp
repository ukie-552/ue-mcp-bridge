#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SUserWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"

class FMcpGetEditorUIOptionsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_editor_ui_options"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ElementPath;
        if (!Params->TryGetStringField(TEXT("element_path"), ElementPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'element_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ElementType;
        Params->TryGetStringField(TEXT("element_type"), ElementType);

        TArray<FString> Options;
        FString CurrentSelection;
        bool bFound = false;

        TSharedRef<SWindow> MainWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
        if (MainWindow.IsValid())
        {
            FWidgetPath WidgetPath;
            if (FSlateApplication::Get().FindWidgetPath(WidgetPath, ElementPath, 0))
            {
                if (WidgetPath.IsValid())
                {
                    bFound = true;

                    TArray<TSharedPtr<FJsonValue>> OptionsArray;
                    for (const FString& Option : Options)
                    {
                        OptionsArray.Add(MakeShareable(new FJsonValueString(Option)));
                    }

                    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
                    Result->SetBoolField(TEXT("found"), bFound);
                    Result->SetStringField(TEXT("element_path"), ElementPath);
                    Result->SetArrayField(TEXT("options"), OptionsArray);
                    Result->SetStringField(TEXT("current_selection"), CurrentSelection);
                    Result->SetNumberField(TEXT("option_count"), Options.Num());

                    return FMcpCommandResult::Success(Result);
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("found"), bFound);
        Result->SetStringField(TEXT("element_path"), ElementPath);

        if (!bFound)
        {
            Result->SetStringField(TEXT("message"), TEXT("Element not found"));
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetEditorUIOptionsHandler)