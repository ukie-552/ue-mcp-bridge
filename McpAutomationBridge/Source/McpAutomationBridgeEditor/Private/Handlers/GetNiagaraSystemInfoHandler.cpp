#include "CoreMinimal.h"
#include "McpCommand.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"

class FMcpGetNiagaraSystemInfoHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_niagara_system_info"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SystemPath;
        if (!Params->TryGetStringField(TEXT("system_path"), SystemPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'system_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
        if (!NiagaraSystem)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("system_name"), NiagaraSystem->GetName());
        Result->SetStringField(TEXT("system_path"), SystemPath);

        TArray<TSharedPtr<FJsonValue>> EmittersArray;
        const TArray<FNiagaraEmitterHandle>& EmitterHandles = NiagaraSystem->GetEmitterHandles();
        for (const FNiagaraEmitterHandle& Handle : EmitterHandles)
        {
            TSharedPtr<FJsonObject> EmitterInfo = MakeShareable(new FJsonObject);
            EmitterInfo->SetStringField(TEXT("name"), Handle.GetName().ToString());
            EmitterInfo->SetStringField(TEXT("unique_name"), Handle.GetUniqueEmitterName());
            EmitterInfo->SetStringField(TEXT("id"), Handle.GetId().ToString());
            EmitterInfo->SetBoolField(TEXT("is_enabled"), Handle.IsEnabled());
            EmitterInfo->SetBoolField(TEXT("is_solo"), Handle.IsSolo());

            UNiagaraEmitter* Emitter = Handle.GetInstance();
            if (Emitter)
            {
                EmitterInfo->SetStringField(TEXT("emitter_type"), Emitter->GetEmitterName().ToString());
            }

            EmittersArray.Add(MakeShareable(new FJsonValueObject(EmitterInfo)));
        }
        Result->SetArrayField(TEXT("emitters"), EmittersArray);
        Result->SetNumberField(TEXT("emitter_count"), EmitterHandles.Num());

        TArray<TSharedPtr<FJsonValue>> ParametersArray;
        Result->SetArrayField(TEXT("parameters"), ParametersArray);
        Result->SetNumberField(TEXT("parameter_count"), 0);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetNiagaraSystemInfoHandler)
