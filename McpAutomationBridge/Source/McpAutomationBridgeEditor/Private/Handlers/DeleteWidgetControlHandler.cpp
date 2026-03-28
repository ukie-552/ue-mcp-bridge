#include "CoreMinimal.h"
#include "McpCommand.h"
#include "WidgetBlueprint.h"
#include "Components/Widget.h"
#include "Components/PanelWidget.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpDeleteWidgetControlHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("delete_widget_control"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString WidgetPath;
        if (!Params->TryGetStringField(TEXT("widget_path"), WidgetPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'widget_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ControlName;
        if (!Params->TryGetStringField(TEXT("control_name"), ControlName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'control_name' parameter"), TEXT("MISSING_PARAM"));
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

        UWidget* Control = WidgetTree->FindWidget(*ControlName);
        if (!Control)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Control not found: %s"), *ControlName),
                TEXT("CONTROL_NOT_FOUND")
            );
        }

        FString ControlType = Control->GetClass()->GetName();
        FString ParentName;

        if (Control->Slot)
        {
            UPanelWidget* ParentPanel = Cast<UPanelWidget>(Control->Slot->Parent);
            if (ParentPanel)
            {
                ParentName = ParentPanel->GetName();
            }
        }

        if (WidgetBP->WidgetTree->RootWidget == Control)
        {
            WidgetBP->WidgetTree->RootWidget = nullptr;
        }

        Control->RemoveFromParent();
        Control->MarkAsGarbage();

        FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("deleted"), true);
        Result->SetStringField(TEXT("control_name"), ControlName);
        Result->SetStringField(TEXT("control_type"), ControlType);

        if (!ParentName.IsEmpty())
        {
            Result->SetStringField(TEXT("parent_control"), ParentName);
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpDeleteWidgetControlHandler)
