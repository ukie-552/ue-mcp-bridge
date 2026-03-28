#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorUndoClient.h"

class FMcpEditorUndoRedoHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("editor_undo_redo"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Action;
        if (!Params->TryGetStringField(TEXT("action"), Action))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'action' parameter (undo/redo)"), TEXT("MISSING_PARAM"));
        }

        int32 Count = 1;
        Params->TryGetNumberField(TEXT("count"), Count);
        Count = FMath::Max(1, Count);

        bool bSuccess = false;
        FString Message;

        if (Action == TEXT("undo"))
        {
            for (int32 i = 0; i < Count; i++)
            {
                if (GEditor->UndoTransaction())
                {
                    bSuccess = true;
                }
                else
                {
                    break;
                }
            }
            Message = FString::Printf(TEXT("Performed %d undo(s)"), Count);
        }
        else if (Action == TEXT("redo"))
        {
            for (int32 i = 0; i < Count; i++)
            {
                if (GEditor->RedoTransaction())
                {
                    bSuccess = true;
                }
                else
                {
                    break;
                }
            }
            Message = FString::Printf(TEXT("Performed %d redo(s)"), Count);
        }
        else
        {
            return FMcpCommandResult::Failure(TEXT("Invalid action. Use 'undo' or 'redo'"), TEXT("INVALID_ACTION"));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("success"), bSuccess);
        Result->SetStringField(TEXT("action"), Action);
        Result->SetNumberField(TEXT("count"), Count);
        Result->SetStringField(TEXT("message"), Message);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpEditorUndoRedoHandler)
