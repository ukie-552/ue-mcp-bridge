#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FMcpAutomationBridgeModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    virtual bool SupportsDynamicReloading() override { return true; }

    static FMcpAutomationBridgeModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FMcpAutomationBridgeModule>(TEXT("McpAutomationBridge"));
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded(TEXT("McpAutomationBridge"));
    }

private:
    bool bModuleInitialized = false;
};
