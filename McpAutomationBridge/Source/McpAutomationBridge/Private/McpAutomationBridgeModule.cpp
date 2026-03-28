#include "McpAutomationBridgeModule.h"

#define LOCTEXT_NAMESPACE "FMcpAutomationBridgeModule"

void FMcpAutomationBridgeModule::StartupModule()
{
    if (bModuleInitialized)
    {
        return;
    }
    
    bModuleInitialized = true;
}

void FMcpAutomationBridgeModule::ShutdownModule()
{
    if (!bModuleInitialized)
    {
        return;
    }
    
    bModuleInitialized = false;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMcpAutomationBridgeModule, McpAutomationBridge)
