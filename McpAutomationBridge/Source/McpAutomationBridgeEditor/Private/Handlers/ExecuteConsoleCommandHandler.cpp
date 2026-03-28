#include "CoreMinimal.h"
#include "McpCommand.h"
#include "HAL/IConsoleManager.h"
#include "Editor.h"

class FMcpExecuteConsoleCommandHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("execute_console_command"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Command;
        if (!Params->TryGetStringField(TEXT("command"), Command))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'command' parameter"), TEXT("MISSING_PARAM"));
        }

        TArray<FString> BlockedCommands = {
            TEXT("quit"),
            TEXT("exit"),
            TEXT("shutdown"),
            TEXT("crash"),
            TEXT("debug.crash"),
            TEXT("obj gc"),
            TEXT("gc.collectall")
        };

        FString LowerCommand = Command.ToLower();
        for (const FString& Blocked : BlockedCommands)
        {
            if (LowerCommand.StartsWith(Blocked))
            {
                return FMcpCommandResult::Failure(
                    FString::Printf(TEXT("Blocked dangerous command: %s"), *Command),
                    TEXT("BLOCKED_COMMAND")
                );
            }
        }

        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World)
        {
            return FMcpCommandResult::Failure(TEXT("No world available"), TEXT("NO_WORLD"));
        }

        GEngine->Exec(World, *Command);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("executed"), true);
        Result->SetStringField(TEXT("command"), Command);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpExecuteConsoleCommandHandler)
