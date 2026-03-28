#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "FileHelpers.h"

class FMcpTakeScreenshotHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("take_screenshot"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString OutputPath;
        if (!Params->TryGetStringField(TEXT("output_path"), OutputPath))
        {
            OutputPath = FPaths::ProjectDir() / TEXT("Screenshots") / FString::Printf(TEXT("Screenshot_%s.png"), *FDateTime::Now().ToString());
        }

        int32 Width = 1920;
        int32 Height = 1080;
        Params->TryGetNumberField(TEXT("width"), Width);
        Params->TryGetNumberField(TEXT("height"), Height);

        bool bShowUI = false;
        Params->TryGetBoolField(TEXT("show_ui"), bShowUI);

        FHighResScreenshotConfig& HighResConfig = GetHighResScreenshotConfig();
        HighResConfig.SetResolution(FIntVector(Width, Height, 0));
        HighResConfig.bCaptureHDR = false;
        HighResConfig.bDumpBufferVisualizationTargets = false;

        FString AbsolutePath = FPaths::ConvertRelativePathToFull(OutputPath);
        FPaths::MakeStandardFilename(AbsolutePath);

        FScreenshotRequest::RequestScreenshot(AbsolutePath, bShowUI, false);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("requested"), true);
        Result->SetStringField(TEXT("output_path"), OutputPath);
        Result->SetNumberField(TEXT("width"), Width);
        Result->SetNumberField(TEXT("height"), Height);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpTakeScreenshotHandler)
