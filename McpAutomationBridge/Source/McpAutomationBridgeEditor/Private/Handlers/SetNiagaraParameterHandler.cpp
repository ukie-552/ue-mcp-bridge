#include "CoreMinimal.h"
#include "McpCommand.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"

class FMcpSetNiagaraParameterHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_niagara_parameter"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SystemPath;
        if (!Params->TryGetStringField(TEXT("system_path"), SystemPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'system_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ParameterName;
        if (!Params->TryGetStringField(TEXT("parameter_name"), ParameterName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'parameter_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ParameterType;
        if (!Params->TryGetStringField(TEXT("parameter_type"), ParameterType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'parameter_type' parameter"), TEXT("MISSING_PARAM"));
        }

        UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
        if (!NiagaraSystem)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        NiagaraSystem->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), true);
        Result->SetStringField(TEXT("parameter_name"), ParameterName);
        Result->SetStringField(TEXT("parameter_type"), ParameterType);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSetNiagaraParameterHandler)
