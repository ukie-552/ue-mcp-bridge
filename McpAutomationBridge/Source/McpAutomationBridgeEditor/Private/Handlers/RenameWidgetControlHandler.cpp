#include "CoreMinimal.h"
#include "McpCommand.h"
#include "WidgetBlueprint.h"
#include "Components/Widget.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpRenameWidgetControlHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("rename_widget_control"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString WidgetPath;
        if (!Params->TryGetStringField(TEXT("widget_path"), WidgetPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'widget_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString OldName;
        if (!Params->TryGetStringField(TEXT("old_name"), OldName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'old_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NewName;
        if (!Params->TryGetStringField(TEXT("new_name"), NewName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'new_name' parameter"), TEXT("MISSING_PARAM"));
        }

        UWidgetBlueprint* WidgetBP = LoadObject<UWidgetBlueprint>(nullptr, *WidgetPath);
        if (!WidgetBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Widget blueprint not found: %s"), *WidgetPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
        if (!WidgetTree)
        {
            return FMcpCommandResult::Failure(TEXT("Widget tree not found"), TEXT("NO_WIDGET_TREE"));
        }

        UWidget* Control = WidgetTree->FindWidget(*OldName);
        if (!Control)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Control not found: %s"), *OldName),
                TEXT("CONTROL_NOT_FOUND")
            );
        }

        UWidget* ExistingControl = WidgetTree->FindWidget(*NewName);
        if (ExistingControl)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Control with name '%s' already exists"), *NewName),
                TEXT("NAME_CONFLICT")
            );
        }

        FString OldControlName = Control->GetName();
        Control->Rename(*NewName);

        FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("renamed"), true);
        Result->SetStringField(TEXT("old_name"), OldControlName);
        Result->SetStringField(TEXT("new_name"), NewName);
        Result->SetStringField(TEXT("control_type"), Control->GetClass()->GetName());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpRenameWidgetControlHandler)
