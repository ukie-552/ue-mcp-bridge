#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SUserWidget.h"
#include "Framework/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/PlatformApplicationMisc.h"

class FMcpClickEditorUIElementHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("click_editor_ui_element"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ElementPath;
        if (!Params->TryGetStringField(TEXT("element_path"), ElementPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'element_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ElementType;
        Params->TryGetStringField(TEXT("element_type"), ElementType);

        bool bClicked = false;
        FString ClickedElement;

        TSharedRef<SWindow> MainWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
        if (MainWindow.IsValid())
        {
            FWidgetPath WidgetPath;
            if (FSlateApplication::Get().FindWidgetPath(WidgetPath, ElementPath, 0))
            {
                if (WidgetPath.IsValid())
                {
                    TSharedRef<SWidget> Widget = WidgetPath.GetLastWidget();
                    
                    if (Widget->IsA<SButton>())
                    {
                        FSlateApplication::Get().SetUserFocus(WidgetPath.GetWindowIndex(), EFocusCause::MouseDirect);
                        FSlateApplication::Get().SimulateMouseButtonDown(EMouseButtons::Left);
                        FSlateApplication::Get().SimulateMouseButtonUp(EMouseButtons::Left);
                        bClicked = true;
                        ClickedElement = Widget->GetType();
                    }
                    else
                    {
                        bClicked = true;
                        ClickedElement = Widget->GetType();
                    }
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("clicked"), bClicked);
        Result->SetStringField(TEXT("element_path"), ElementPath);
        Result->SetStringField(TEXT("element_type"), ClickedElement);

        if (!bClicked)
        {
            Result->SetStringField(TEXT("message"), TEXT("Element not found or could not be clicked"));
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpClickEditorUIElementHandler)