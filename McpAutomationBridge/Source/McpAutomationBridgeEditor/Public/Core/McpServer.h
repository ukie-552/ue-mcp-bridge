#pragma once

#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Containers/Queue.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "SocketSubsystem.h"
#include "Sockets.h"

class FMcpWebSocketServer;

class FMcpServer
{
public:
    static FMcpServer& Get();

    void Initialize();
    void Shutdown();
    
    bool IsRunning() const { return bIsRunning; }
    int32 GetPort() const { return Port; }
    void SetPort(int32 InPort) { Port = InPort; }

    void ProcessCommand(const FString& CommandJson, FString& OutResponse);

private:
    FMcpServer() = default;
    ~FMcpServer() = default;

    bool bIsRunning = false;
    int32 Port = 8091;
    FString Host = TEXT("127.0.0.1");

    TSharedPtr<FMcpWebSocketServer> WebSocketServer;

    void HandleClientMessage(const FString& Message, FString& OutResponse);
    TSharedPtr<FJsonObject> ParseCommand(const FString& JsonString);
};
