#include "Core/LogCapture.h"
#include "McpCommand.h"

class FGetEngineLogsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override
    {
        return TEXT("get_engine_logs");
    }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        int32 MaxCount = 100;
        if (Params->HasField(TEXT("max_count")))
        {
            MaxCount = Params->GetNumberField(TEXT("max_count"));
        }
        MaxCount = FMath::Clamp(MaxCount, 1, 1000);

        TArray<FLogEntry> Logs;
        FLogCapture::Get().GetRecentLogs(MaxCount, Logs);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        TArray<TSharedPtr<FJsonValue>> LogArray;

        for (const FLogEntry& Log : Logs)
        {
            TSharedPtr<FJsonObject> LogObj = MakeShareable(new FJsonObject);
            LogObj->SetStringField(TEXT("timestamp"), Log.Timestamp.ToString());
            LogObj->SetStringField(TEXT("category"), Log.Category);
            LogObj->SetStringField(TEXT("verbosity"), UEnum::GetValueAsString(Log.Verbosity));
            LogObj->SetStringField(TEXT("message"), Log.Message);
            LogArray.Add(MakeShareable(new FJsonValueObject(LogObj)));
        }

        Result->SetArrayField(TEXT("logs"), LogArray);
        Result->SetNumberField(TEXT("count"), LogArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FGetEngineLogsHandler);
