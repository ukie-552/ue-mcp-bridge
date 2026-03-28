#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FMcpAutomationBridgeEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    virtual bool SupportsDynamicReloading() override { return true; }

    static FMcpAutomationBridgeEditorModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FMcpAutomationBridgeEditorModule>(TEXT("McpAutomationBridgeEditor"));
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded(TEXT("McpAutomationBridgeEditor"));
    }

private:
    bool bModuleInitialized = false;
};
