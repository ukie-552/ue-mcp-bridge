#include "Core/LogCapture.h"
#include "McpCommand.h"

class FMcpGetEngineLogsHandler : public FMcpCommandHandler
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
            TSharedPtr<FJsonObject> LogEntryObj = MakeShareable(new FJsonObject);
            LogEntryObj->SetStringField(TEXT("timestamp"), Log.Timestamp.ToString());
            LogEntryObj->SetStringField(TEXT("category"), Log.Category);
            
            FString VerbosityStr;
            switch (Log.Verbosity)
            {
                case ELogVerbosity::Fatal: VerbosityStr = TEXT("Fatal"); break;
                case ELogVerbosity::Error: VerbosityStr = TEXT("Error"); break;
                case ELogVerbosity::Warning: VerbosityStr = TEXT("Warning"); break;
                case ELogVerbosity::Display: VerbosityStr = TEXT("Display"); break;
                case ELogVerbosity::Log: VerbosityStr = TEXT("Log"); break;
                case ELogVerbosity::Verbose: VerbosityStr = TEXT("Verbose"); break;
                case ELogVerbosity::VeryVerbose: VerbosityStr = TEXT("VeryVerbose"); break;
                default: VerbosityStr = TEXT("Unknown"); break;
            }
            LogEntryObj->SetStringField(TEXT("verbosity"), VerbosityStr);
            LogEntryObj->SetStringField(TEXT("message"), Log.Message);
            LogArray.Add(MakeShareable(new FJsonValueObject(LogEntryObj)));
        }

        Result->SetArrayField(TEXT("logs"), LogArray);
        Result->SetNumberField(TEXT("count"), LogArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetEngineLogsHandler);
