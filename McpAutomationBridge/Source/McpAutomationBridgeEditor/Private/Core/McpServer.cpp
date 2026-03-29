#include "Core/McpServer.h"
#include "Core/LogCapture.h"
#include "WebSocket/McpWebSocketServer.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Policies/CondensedJsonPrintPolicy.h"

FMcpServer& FMcpServer::Get()
{
    static FMcpServer Instance;
    return Instance;
}

void FMcpServer::Initialize()
{
    if (bIsRunning)
    {
        return;
    }

    FLogCapture::Get().StartCapture();

    WebSocketServer = MakeShareable(new FMcpWebSocketServer());
    if (WebSocketServer->Start(Host, Port))
    {
        bIsRunning = true;
        UE_LOG(LogTemp, Log, TEXT("MCP Server started on %s:%d"), *Host, Port);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to start MCP Server on %s:%d"), *Host, Port);
    }
}

void FMcpServer::Shutdown()
{
    if (!bIsRunning)
    {
        return;
    }

    FLogCapture::Get().StopCapture();

    if (WebSocketServer.IsValid())
    {
        WebSocketServer->Stop();
        WebSocketServer.Reset();
    }

    bIsRunning = false;
    UE_LOG(LogTemp, Log, TEXT("MCP Server stopped"));
}

void FMcpServer::ProcessCommand(const FString& CommandJson, FString& OutResponse)
{
    TSharedPtr<FJsonObject> Command = ParseCommand(CommandJson);
    
    if (!Command.IsValid())
    {
        FMcpCommandResult Result = FMcpCommandResult::Failure(
            TEXT("Invalid JSON command"),
            TEXT("INVALID_JSON")
        );
        OutResponse = Result.ToJsonString();
        return;
    }

    FString CommandName;
    if (!Command->TryGetStringField(TEXT("command"), CommandName))
    {
        FMcpCommandResult Result = FMcpCommandResult::Failure(
            TEXT("Missing 'command' field"),
            TEXT("MISSING_COMMAND")
        );
        OutResponse = Result.ToJsonString();
        return;
    }

    const TSharedPtr<FJsonObject>* ParamsPtr = nullptr;
    TSharedPtr<FJsonObject> Params;
    if (Command->TryGetObjectField(TEXT("params"), ParamsPtr))
    {
        Params = *ParamsPtr;
    }
    else
    {
        Params = MakeShareable(new FJsonObject);
    }

    FMcpCommandResult Result = IMcpCommandRegistry::ExecuteCommand(CommandName, Params);
    OutResponse = Result.ToJsonString();
}

TSharedPtr<FJsonObject> FMcpServer::ParseCommand(const FString& JsonString)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        return nullptr;
    }
    
    return JsonObject;
}
