#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

struct FMcpCommandResult
{
    bool bSuccess = false;
    TSharedPtr<FJsonObject> Result;
    FString Error;
    FString ErrorCode;

    static FMcpCommandResult Success(TSharedPtr<FJsonObject> InResult = nullptr)
    {
        FMcpCommandResult R;
        R.bSuccess = true;
        R.Result = InResult;
        return R;
    }

    static FMcpCommandResult Failure(const FString& InError, const FString& InCode = TEXT("UNKNOWN_ERROR"))
    {
        FMcpCommandResult R;
        R.bSuccess = false;
        R.Error = InError;
        R.ErrorCode = InCode;
        return R;
    }

    FString ToJsonString() const
    {
        TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject);
        
        if (bSuccess)
        {
            RootObj->SetBoolField(TEXT("success"), true);
            if (Result.IsValid())
            {
                RootObj->SetObjectField(TEXT("result"), Result);
            }
        }
        else
        {
            RootObj->SetBoolField(TEXT("success"), false);
            RootObj->SetStringField(TEXT("error"), Error);
            RootObj->SetStringField(TEXT("error_code"), ErrorCode);
        }

        FString OutputString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
        FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);
        return OutputString;
    }
};

class FMcpCommandHandler
{
public:
    virtual ~FMcpCommandHandler() = default;
    virtual FString GetCommandName() const = 0;
    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) = 0;
};

class MCPAUTOMATIONBRIDGE_API IMcpCommandRegistry
{
public:
    static TMap<FString, TSharedPtr<FMcpCommandHandler>>& GetHandlers()
    {
        static TMap<FString, TSharedPtr<FMcpCommandHandler>> Handlers;
        return Handlers;
    }

    static void RegisterHandler(TSharedPtr<FMcpCommandHandler> Handler)
    {
        if (Handler.IsValid())
        {
            GetHandlers().Add(Handler->GetCommandName(), Handler);
        }
    }

    static FMcpCommandResult ExecuteCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params)
    {
        TSharedPtr<FMcpCommandHandler>* Handler = GetHandlers().Find(CommandName);
        if (Handler && Handler->IsValid())
        {
            return (*Handler)->Execute(Params);
        }
        return FMcpCommandResult::Failure(
            FString::Printf(TEXT("Unknown command: %s"), *CommandName),
            TEXT("UNKNOWN_COMMAND")
        );
    }
};

template<typename THandler>
class TMcpCommandRegistrar
{
public:
    TMcpCommandRegistrar()
    {
        IMcpCommandRegistry::RegisterHandler(MakeShareable(new THandler()));
    }
};

#define REGISTER_MCP_COMMAND(HandlerClass) \
    static TMcpCommandRegistrar<HandlerClass> _##HandlerClass##Registrar;
