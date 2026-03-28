#pragma once

#include "CoreMinimal.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "HttpRouteHandle.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"

class FMcpWebSocketServer
{
public:
    FMcpWebSocketServer();
    ~FMcpWebSocketServer();

    bool Start(const FString& InHost, int32 InPort);
    void Stop();
    bool IsRunning() const { return bIsRunning; }

private:
    bool bIsRunning = false;
    FString Host;
    int32 Port = 8091;
    
    TSharedPtr<IHttpRouter> HttpRouter;
    FHttpRouteHandle McpRouteHandle;

    TSharedPtr<FHttpServerResponse> HandleMcpRequest(const FHttpServerRequest& Request);
    void BroadcastMessage(const FString& Message);
};
