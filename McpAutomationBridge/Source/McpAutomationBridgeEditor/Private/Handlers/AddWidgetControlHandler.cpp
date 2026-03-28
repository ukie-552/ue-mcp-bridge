#include "CoreMinimal.h"
#include "McpCommand.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintEditor.h"
#include "Components/Widget.h"
#include "Components/CanvasPanel.h"
#include "Components/Overlay.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/ProgressBar.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/InvalidationBox.h"
#include "Components/RetainerBox.h"
#include "Components/SafeZone.h"
#include "Components/ScaleBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/WrapBox.h"
#include "Editor/WidgetCompilerLog.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"

class FMcpAddWidgetControlHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_widget_control"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString WidgetPath;
        if (!Params->TryGetStringField(TEXT("widget_path"), WidgetPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'widget_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ControlType;
        if (!Params->TryGetStringField(TEXT("control_type"), ControlType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'control_type' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ControlName;
        Params->TryGetStringField(TEXT("control_name"), ControlName);

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

        UWidget* NewControl = CreateControlByType(ControlType, WidgetTree);
        if (!NewControl)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create control of type: %s"), *ControlType),
                TEXT("CONTROL_CREATION_FAILED")
            );
        }

        if (!ControlName.IsEmpty())
        {
            NewControl->Rename(*ControlName);
        }

        UPanelWidget* ParentPanel = nullptr;
        if (!ParentControlName.IsEmpty())
        {
            ParentPanel = Cast<UPanelWidget>(WidgetTree->FindWidget(*ParentControlName));
        }

        if (!ParentPanel)
        {
            ParentPanel = Cast<UPanelWidget>(WidgetTree->RootWidget.Get());
        }

        if (ParentPanel)
        {
            ParentPanel->AddChild(NewControl);
        }
        else
        {
            if (!WidgetTree->RootWidget)
            {
                UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
                RootCanvas->Rename(TEXT("RootCanvas"));
                WidgetTree->RootWidget = RootCanvas;
                RootCanvas->AddChild(NewControl);
            }
            else
            {
                UPanelWidget* RootPanel = Cast<UPanelWidget>(WidgetTree->RootWidget.Get());
                if (RootPanel)
                {
                    RootPanel->AddChild(NewControl);
                }
            }
        }

        FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("added"), true);
        Result->SetStringField(TEXT("control_name"), NewControl->GetName());
        Result->SetStringField(TEXT("control_type"), NewControl->GetClass()->GetName());
        Result->SetStringField(TEXT("widget_path"), WidgetPath);

        if (ParentPanel)
        {
            Result->SetStringField(TEXT("parent_control"), ParentPanel->GetName());
        }

        return FMcpCommandResult::Success(Result);
    }

private:
    UWidget* CreateControlByType(const FString& ControlType, UWidgetTree* WidgetTree)
    {
        if (!WidgetTree)
        {
            return nullptr;
        }

        static TMap<FString, TSubclassOf<UWidget>> ControlTypes = {
            {TEXT("CanvasPanel"), UCanvasPanel::StaticClass()},
            {TEXT("Overlay"), UOverlay::StaticClass()},
            {TEXT("VerticalBox"), UVerticalBox::StaticClass()},
            {TEXT("HorizontalBox"), UHorizontalBox::StaticClass()},
            {TEXT("Border"), UBorder::StaticClass()},
            {TEXT("Image"), UImage::StaticClass()},
            {TEXT("TextBlock"), UTextBlock::StaticClass()},
            {TEXT("Button"), UButton::StaticClass()},
            {TEXT("CheckBox"), UCheckBox::StaticClass()},
            {TEXT("Slider"), USlider::StaticClass()},
            {TEXT("ProgressBar"), UProgressBar::StaticClass()},
            {TEXT("EditableText"), UEditableText::StaticClass()},
            {TEXT("EditableTextBox"), UEditableTextBox::StaticClass()},
            {TEXT("ComboBoxString"), UComboBoxString::StaticClass()},
            {TEXT("ListView"), UListView::StaticClass()},
            {TEXT("ScrollBox"), UScrollBox::StaticClass()},
            {TEXT("SizeBox"), USizeBox::StaticClass()},
            {TEXT("Spacer"), USpacer::StaticClass()},
            {TEXT("InvalidationBox"), UInvalidationBox::StaticClass()},
            {TEXT("RetainerBox"), URetainerBox::StaticClass()},
            {TEXT("SafeZone"), USafeZone::StaticClass()},
            {TEXT("ScaleBox"), UScaleBox::StaticClass()},
            {TEXT("UniformGridPanel"), UUniformGridPanel::StaticClass()},
            {TEXT("WrapBox"), UWrapBox::StaticClass()},
        };

        TSubclassOf<UWidget>* WidgetClass = ControlTypes.Find(ControlType);
        if (WidgetClass && *WidgetClass)
        {
            return WidgetTree->ConstructWidget<UWidget>(*WidgetClass);
        }

        UClass* CustomClass = LoadClass<UWidget>(nullptr, *ControlType);
        if (CustomClass)
        {
            return WidgetTree->ConstructWidget<UWidget>(CustomClass);
        }

        return nullptr;
    }
};

REGISTER_MCP_COMMAND(FMcpAddWidgetControlHandler)
