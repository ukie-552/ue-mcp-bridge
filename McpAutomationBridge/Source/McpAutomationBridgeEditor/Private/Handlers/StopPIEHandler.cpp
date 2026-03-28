#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "PlayLevel.h"

class FMcpStopPIEHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("stop_pie"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        if (!GEditor->IsPlayingSessionInEditor())
        {
            return FMcpCommandResult::Failure(TEXT("No PIE session is currently running"), TEXT("NO_PIE_SESSION"));
        }

        GEditor->RequestEndPlayMap();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("stopped"), true);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpStopPIEHandler)
