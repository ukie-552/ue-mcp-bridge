#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "PlayInEditorDataTypes.h"
#include "LevelEditor.h"
#include "Settings/LevelEditorPlaySettings.h"

class FMcpStartPIEHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("start_pie"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        bool bSimulateInEditor = false;
        Params->TryGetBoolField(TEXT("simulate_in_editor"), bSimulateInEditor);

        FRequestPlaySessionParams PlaySessionParams;
        
        if (bSimulateInEditor)
        {
            PlaySessionParams.WorldType = EPlaySessionWorldType::SimulateInEditor;
        }
        else
        {
            PlaySessionParams.WorldType = EPlaySessionWorldType::PlayInEditor;
        }

        GEditor->RequestPlaySession(PlaySessionParams);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("started"), true);
        Result->SetBoolField(TEXT("simulate_in_editor"), bSimulateInEditor);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpStartPIEHandler)
