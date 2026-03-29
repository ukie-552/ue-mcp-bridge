#include "WebSocket/McpWebSocketServer.h"
#include "Core/McpServer.h"
#include "HttpServerConstants.h"
#include "HttpPath.h"

FMcpWebSocketServer::FMcpWebSocketServer()
{
}

FMcpWebSocketServer::~FMcpWebSocketServer()
{
    Stop();
}

bool FMcpWebSocketServer::Start(const FString& InHost, int32 InPort)
{
    if (bIsRunning)
    {
        return true;
    }

    Host = InHost;
    Port = InPort;

    FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
    HttpRouter = HttpServerModule.GetHttpRouter(Port);
    
    if (!HttpRouter.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create HTTP router on port %d"), Port);
        return false;
    }

    FHttpPath McpPath(TEXT("/mcp"));
    McpRouteHandle = HttpRouter->BindRoute(
        McpPath,
        EHttpServerRequestVerbs::VERB_POST,
        FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
        {
            TUniquePtr<FHttpServerResponse> Response = HandleMcpRequest(Request);
            OnComplete(MoveTemp(Response));
            return true;
        })
    );

    if (!McpRouteHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to bind MCP route on port %d"), Port);
        HttpRouter.Reset();
        return false;
    }

    HttpServerModule.StartAllListeners();
    bIsRunning = true;
    UE_LOG(LogTemp, Log, TEXT("MCP HTTP server started on %s:%d"), *Host, Port);
    return true;
}

void FMcpWebSocketServer::Stop()
{
    if (!bIsRunning)
    {
        return;
    }

    if (HttpRouter.IsValid() && McpRouteHandle.IsValid())
    {
        HttpRouter->UnbindRoute(McpRouteHandle);
        McpRouteHandle.Reset();
    }

    if (HttpRouter.IsValid())
    {
        FHttpServerModule::Get().StopAllListeners();
        HttpRouter.Reset();
    }

    bIsRunning = false;
    UE_LOG(LogTemp, Log, TEXT("MCP HTTP server stopped"));
}

TUniquePtr<FHttpServerResponse> FMcpWebSocketServer::HandleMcpRequest(const FHttpServerRequest& Request)
{
    FString RequestBody;
    const TArray<uint8>& BodyBytes = Request.Body;
    RequestBody = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(BodyBytes.GetData())));
    
    FString Response;
    FMcpServer::Get().ProcessCommand(RequestBody, Response);
    
    TUniquePtr<FHttpServerResponse> HttpResponse = FHttpServerResponse::Create(Response, TEXT("application/json"));
    HttpResponse->Headers.Add(TEXT("Access-Control-Allow-Origin"), TArray<FString>{TEXT("*")});
    HttpResponse->Headers.Add(TEXT("Access-Control-Allow-Methods"), TArray<FString>{TEXT("POST, OPTIONS")});
    HttpResponse->Headers.Add(TEXT("Access-Control-Allow-Headers"), TArray<FString>{TEXT("Content-Type")});
    
    return HttpResponse;
}

void FMcpWebSocketServer::BroadcastMessage(const FString& Message)
{
}
