#include "McpAutomationBridgeEditorModule.h"
#include "Core/McpServer.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FMcpAutomationBridgeEditorModule"

void FMcpAutomationBridgeEditorModule::StartupModule()
{
    if (bModuleInitialized)
    {
        return;
    }
    
    bModuleInitialized = true;
    
    FMcpServer::Get().Initialize();
}

void FMcpAutomationBridgeEditorModule::ShutdownModule()
{
    if (!bModuleInitialized)
    {
        return;
    }
    
    FMcpServer::Get().Shutdown();
    bModuleInitialized = false;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMcpAutomationBridgeEditorModule, McpAutomationBridgeEditor)
