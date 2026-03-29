#include "CoreMinimal.h"
#include "McpCommand.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraEmitterFactoryNew.h"
#include "AssetRegistry/AssetRegistryModule.h"

class FMcpAddNiagaraEmitterHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_niagara_emitter"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SystemPath;
        if (!Params->TryGetStringField(TEXT("system_path"), SystemPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'system_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString EmitterName;
        if (!Params->TryGetStringField(TEXT("emitter_name"), EmitterName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'emitter_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString EmitterTemplatePath;
        Params->TryGetStringField(TEXT("emitter_template_path"), EmitterTemplatePath);

        bool bIsSolo = false;
        Params->TryGetBoolField(TEXT("is_solo"), bIsSolo);

        bool bIsEnabled = true;
        Params->TryGetBoolField(TEXT("is_enabled"), bIsEnabled);

        UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
        if (!NiagaraSystem)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UNiagaraEmitter* EmitterTemplate = nullptr;
        if (!EmitterTemplatePath.IsEmpty())
        {
            EmitterTemplate = LoadObject<UNiagaraEmitter>(nullptr, *EmitterTemplatePath);
        }

        FNiagaraEmitterHandle EmitterHandle;
        if (EmitterTemplate)
        {
            EmitterHandle = NiagaraSystem->AddEmitterHandle(*EmitterTemplate, *EmitterName, FGuid());
        }
        else
        {
            UNiagaraEmitter* NewEmitter = NewObject<UNiagaraEmitter>(NiagaraSystem, *EmitterName, RF_Public | RF_Transactional);
            if (NewEmitter)
            {
                EmitterHandle = NiagaraSystem->AddEmitterHandle(*NewEmitter, *EmitterName, FGuid());
            }
        }

        if (!EmitterHandle.IsValid())
        {
            return FMcpCommandResult::Failure(TEXT("Failed to add emitter to system"), TEXT("EMITTER_ADD_FAILED"));
        }

        EmitterHandle.SetIsEnabled(bIsEnabled, *NiagaraSystem, true);

        NiagaraSystem->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("added"), true);
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        Result->SetStringField(TEXT("emitter_id"), EmitterHandle.GetId().ToString());
        Result->SetBoolField(TEXT("is_solo"), bIsSolo);
        Result->SetBoolField(TEXT("is_enabled"), bIsEnabled);
        Result->SetNumberField(TEXT("total_emitters"), NiagaraSystem->GetEmitterHandles().Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpAddNiagaraEmitterHandler)
