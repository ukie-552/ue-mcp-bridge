#include "CoreMinimal.h"
#include "McpCommand.h"
#include "WidgetBlueprint.h"
#include "Components/Widget.h"
#include "Components/PanelWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpGetWidgetControlsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_widget_controls"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString WidgetPath;
        if (!Params->TryGetStringField(TEXT("widget_path"), WidgetPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'widget_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bRecursive = true;
        Params->TryGetBoolField(TEXT("recursive"), bRecursive);

        FString ParentControlName;
        Params->TryGetStringField(TEXT("parent_control"), ParentControlName);

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

        TArray<TSharedPtr<FJsonValue>> ControlsArray;

        if (!ParentControlName.IsEmpty())
        {
            UWidget* ParentControl = WidgetTree->FindWidget(*ParentControlName);
            if (!ParentControl)
            {
                return FMcpCommandResult::Failure(
                    FString::Printf(TEXT("Parent control not found: %s"), *ParentControlName),
                    TEXT("CONTROL_NOT_FOUND")
                );
            }

            UPanelWidget* ParentPanel = Cast<UPanelWidget>(ParentControl);
            if (ParentPanel)
            {
                for (UWidget* Child : ParentPanel->GetAllChildren())
                {
                    ControlsArray.Add(WidgetToJson(Child, bRecursive));
                }
            }
        }
        else
        {
            TArray<UWidget*> AllWidgets;
            WidgetTree->GetAllWidgets(AllWidgets);

            for (UWidget* Widget : AllWidgets)
            {
                if (Widget && !Widget->IsA<UPanelWidget>())
                {
                    bool bHasParent = false;
                    if (Widget->Slot)
                    {
                        bHasParent = true;
                    }

                    if (!bHasParent || Widget == WidgetTree->RootWidget)
                    {
                        ControlsArray.Add(WidgetToJson(Widget, bRecursive));
                    }
                }
            }

            if (WidgetTree->RootWidget)
            {
                bool bAlreadyAdded = false;
                for (const auto& JsonValue : ControlsArray)
                {
                    TSharedPtr<FJsonObject> Obj = JsonValue->AsObject();
                    if (Obj && Obj->GetStringField(TEXT("name")) == WidgetTree->RootWidget->GetName())
                    {
                        bAlreadyAdded = true;
                        break;
                    }
                }

                if (!bAlreadyAdded)
                {
                    ControlsArray.Insert(WidgetToJson(WidgetTree->RootWidget, bRecursive), 0);
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("controls"), ControlsArray);
        Result->SetNumberField(TEXT("count"), ControlsArray.Num());

        return FMcpCommandResult::Success(Result);
    }

private:
    TSharedPtr<FJsonValue> WidgetToJson(UWidget* Widget, bool bRecursive)
    {
        if (!Widget)
        {
            return MakeShareable(new FJsonValueNull());
        }

        TSharedPtr<FJsonObject> WidgetObj = MakeShareable(new FJsonObject);
        WidgetObj->SetStringField(TEXT("name"), Widget->GetName());
        WidgetObj->SetStringField(TEXT("type"), Widget->GetClass()->GetName());
        WidgetObj->SetStringField(TEXT("path"), Widget->GetPathName());

        UPanelWidget* PanelWidget = Cast<UPanelWidget>(Widget);
        if (PanelWidget && bRecursive)
        {
            TArray<TSharedPtr<FJsonValue>> ChildrenArray;
            for (UWidget* Child : PanelWidget->GetAllChildren())
            {
                ChildrenArray.Add(WidgetToJson(Child, bRecursive));
            }
            WidgetObj->SetArrayField(TEXT("children"), ChildrenArray);
            WidgetObj->SetNumberField(TEXT("child_count"), ChildrenArray.Num());
        }

        if (Widget->Slot)
        {
            TSharedPtr<FJsonObject> SlotInfo = MakeShareable(new FJsonObject);
            SlotInfo->SetStringField(TEXT("type"), Widget->Slot->GetClass()->GetName());
            WidgetObj->SetObjectField(TEXT("slot"), SlotInfo);
        }

        return MakeShareable(new FJsonValueObject(WidgetObj));
    }
};

REGISTER_MCP_COMMAND(FMcpGetWidgetControlsHandler)
