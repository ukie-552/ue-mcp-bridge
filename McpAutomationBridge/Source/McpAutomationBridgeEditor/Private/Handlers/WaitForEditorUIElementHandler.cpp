#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/ThreadSafeBool.h"
#include "Async/Async.h"

class FMcpWaitForEditorUIElementHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("wait_for_editor_ui_element"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ElementPath;
        if (!Params->TryGetStringField(TEXT("element_path"), ElementPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'element_path' parameter"), TEXT("MISSING_PARAM"));
        }

        double TimeoutSeconds = 5.0;
        Params->TryGetNumberField(TEXT("timeout"), TimeoutSeconds);

        bool bVisible = false;
        double StartTime = FPlatformTime::Seconds();
        double CheckInterval = 0.1;

        while ((FPlatformTime::Seconds() - StartTime) < TimeoutSeconds)
        {
            FWidgetPath WidgetPath;
            if (FSlateApplication::Get().FindWidgetPath(WidgetPath, ElementPath, 0))
            {
                if (WidgetPath.IsValid())
                {
                    bVisible = true;
                    break;
                }
            }

            FPlatformProcess::Sleep(CheckInterval);
        }

        double ElapsedTime = FPlatformTime::Seconds() - StartTime;

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("visible"), bVisible);
        Result->SetStringField(TEXT("element_path"), ElementPath);
        Result->SetNumberField(TEXT("elapsed_time"), ElapsedTime);
        Result->SetNumberField(TEXT("timeout"), TimeoutSeconds);

        if (!bVisible)
        {
            Result->SetStringField(TEXT("message"), TEXT("Element did not appear within timeout period"));
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpWaitForEditorUIElementHandler)