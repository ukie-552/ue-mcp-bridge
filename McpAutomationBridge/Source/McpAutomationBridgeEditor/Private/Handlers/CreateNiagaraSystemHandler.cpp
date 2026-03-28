#include "CoreMinimal.h"
#include "McpCommand.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

class FMcpCreateNiagaraSystemHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_niagara_system"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SystemPath;
        if (!Params->TryGetStringField(TEXT("system_path"), SystemPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'system_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString SystemName;
        if (!Params->TryGetStringField(TEXT("system_name"), SystemName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'system_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString TemplatePath;
        Params->TryGetStringField(TEXT("template_path"), TemplatePath);

        FString FullPath = FString::Printf(TEXT("%s/%s"), *SystemPath, *SystemName);

        FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
        
        UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
            SystemName,
            SystemPath,
            UNiagaraSystem::StaticClass(),
            nullptr
        );

        if (!NewAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create niagara system: %s"), *SystemName),
                TEXT("SYSTEM_CREATION_FAILED")
            );
        }

        UNiagaraSystem* NewSystem = Cast<UNiagaraSystem>(NewAsset);
        if (NewSystem)
        {
            FAssetRegistryModule::AssetCreated(NewSystem);
            NewSystem->MarkPackageDirty();
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("system_name"), SystemName);
        Result->SetStringField(TEXT("system_path"), FullPath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateNiagaraSystemHandler)
