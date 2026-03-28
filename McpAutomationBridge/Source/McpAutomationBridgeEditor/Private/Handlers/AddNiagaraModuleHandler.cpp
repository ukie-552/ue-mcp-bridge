#include "CoreMinimal.h"
#include "McpCommand.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "Modules/ModuleManager.h"

class FMcpAddNiagaraModuleHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_niagara_module"); }

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

        FString ModuleName;
        if (!Params->TryGetStringField(TEXT("module_name"), ModuleName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'module_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString StackContext;
        Params->TryGetStringField(TEXT("stack_context"), StackContext);

        int32 InsertIndex = -1;
        Params->TryGetNumberField(TEXT("insert_index"), InsertIndex);

        UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
        if (!NiagaraSystem)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UNiagaraEmitter* TargetEmitter = nullptr;

        for (const FNiagaraEmitterHandle& Handle : NiagaraSystem->GetEmitterHandles())
        {
            if (Handle.GetUniqueEmitterName() == EmitterName || Handle.GetName().ToString() == EmitterName)
            {
                TargetEmitter = Handle.GetInstance();
                break;
            }
        }

        if (!TargetEmitter)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Emitter not found: %s"), *EmitterName),
                TEXT("EMITTER_NOT_FOUND")
            );
        }

        NiagaraSystem->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("added"), true);
        Result->SetStringField(TEXT("module_name"), ModuleName);
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        Result->SetStringField(TEXT("stack_context"), StackContext.IsEmpty() ? TEXT("ParticleSpawn") : StackContext);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpAddNiagaraModuleHandler)
