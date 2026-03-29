#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Components/Widget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/ProgressBar.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/TileView.h"
#include "Components/TreeView.h"
#include "Components/CanvasPanel.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/GridPanel.h"
#include "Components/UniformGridPanel.h"
#include "Components/Overlay.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/Border.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableText.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/SpinBox.h"
#include "Components/RichTextBlock.h"
#include "Components/Throbber.h"
#include "Components/CircularThrobber.h"
#include "Components/ExpandableArea.h"
#include "Components/WrapBox.h"
#include "Components/InvalidationBox.h"
#include "Components/RetainerBox.h"
#include "Components/SafeZone.h"
#include "Components/ScaleBox.h"
#include "UObject/UObjectHash.h"

class FMcpGetAvailableWidgetTypesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_available_widget_types"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Category;
        Params->TryGetStringField(TEXT("category"), Category);

        TArray<TSharedPtr<FJsonValue>> WidgetTypesArray;

        TMap<FString, TArray<FString>> WidgetCategories;

        {
            TArray<FString> BasicWidgets;
            BasicWidgets.Add(TEXT("TextBlock"));
            BasicWidgets.Add(TEXT("Image"));
            BasicWidgets.Add(TEXT("Button"));
            BasicWidgets.Add(TEXT("CheckBox"));
            BasicWidgets.Add(TEXT("ProgressBar"));
            BasicWidgets.Add(TEXT("Slider"));
            BasicWidgets.Add(TEXT("SpinBox"));
            BasicWidgets.Add(TEXT("Border"));
            BasicWidgets.Add(TEXT("Spacer"));
            WidgetCategories.Add(TEXT("Basic"), BasicWidgets);
        }

        {
            TArray<FString> InputWidgets;
            InputWidgets.Add(TEXT("EditableText"));
            InputWidgets.Add(TEXT("EditableTextBox"));
            InputWidgets.Add(TEXT("MultiLineEditableText"));
            InputWidgets.Add(TEXT("MultiLineEditableTextBox"));
            InputWidgets.Add(TEXT("ComboBoxString"));
            InputWidgets.Add(TEXT("NumericUpDown"));
            WidgetCategories.Add(TEXT("Input"), InputWidgets);
        }

        {
            TArray<FString> PanelWidgets;
            PanelWidgets.Add(TEXT("CanvasPanel"));
            PanelWidgets.Add(TEXT("HorizontalBox"));
            PanelWidgets.Add(TEXT("VerticalBox"));
            PanelWidgets.Add(TEXT("GridPanel"));
            PanelWidgets.Add(TEXT("UniformGridPanel"));
            PanelWidgets.Add(TEXT("Overlay"));
            PanelWidgets.Add(TEXT("ScrollBox"));
            PanelWidgets.Add(TEXT("SizeBox"));
            PanelWidgets.Add(TEXT("ScaleBox"));
            PanelWidgets.Add(TEXT("WrapBox"));
            PanelWidgets.Add(TEXT("InvalidationBox"));
            PanelWidgets.Add(TEXT("RetainerBox"));
            PanelWidgets.Add(TEXT("SafeZone"));
            WidgetCategories.Add(TEXT("Panels"), PanelWidgets);
        }

        {
            TArray<FString> ListWidgets;
            ListWidgets.Add(TEXT("ListView"));
            ListWidgets.Add(TEXT("TileView"));
            ListWidgets.Add(TEXT("TreeView"));
            WidgetCategories.Add(TEXT("Lists"), ListWidgets);
        }

        {
            TArray<FString> AdvancedWidgets;
            AdvancedWidgets.Add(TEXT("RichTextBlock"));
            AdvancedWidgets.Add(TEXT("Throbber"));
            AdvancedWidgets.Add(TEXT("CircularThrobber"));
            AdvancedWidgets.Add(TEXT("ExpandableArea"));
            WidgetCategories.Add(TEXT("Advanced"), AdvancedWidgets);
        }

        for (const auto& CategoryPair : WidgetCategories)
        {
            if (Category.IsEmpty() || Category == CategoryPair.Key)
            {
                for (const FString& WidgetType : CategoryPair.Value)
                {
                    TSharedPtr<FJsonObject> WidgetInfo = MakeShareable(new FJsonObject);
                    WidgetInfo->SetStringField(TEXT("name"), WidgetType);
                    WidgetInfo->SetStringField(TEXT("category"), CategoryPair.Key);
                    WidgetInfo->SetStringField(TEXT("class_path"), FString::Printf(TEXT("/Script/UMGEditor.%s"), *WidgetType));
                    WidgetTypesArray.Add(MakeShareable(new FJsonValueObject(WidgetInfo)));
                }
            }
        }

        TArray<UClass*> AllWidgetClasses;
        ::GetDerivedClasses(UWidget::StaticClass(), AllWidgetClasses, true);

        TSet<FString> AlreadyAddedTypes;
        for (const auto& JsonValue : WidgetTypesArray)
        {
            AlreadyAddedTypes.Add(JsonValue->AsObject()->GetStringField(TEXT("name")));
        }

        for (UClass* WidgetClass : AllWidgetClasses)
        {
            if (WidgetClass && !WidgetClass->HasAnyClassFlags(CLASS_Abstract))
            {
                FString ClassName = WidgetClass->GetName();
                if (!AlreadyAddedTypes.Contains(ClassName))
                {
                    TSharedPtr<FJsonObject> WidgetInfo = MakeShareable(new FJsonObject);
                    WidgetInfo->SetStringField(TEXT("name"), ClassName);
                    WidgetInfo->SetStringField(TEXT("category"), TEXT("All"));
                    WidgetInfo->SetStringField(TEXT("class_path"), WidgetClass->GetPathName());
                    WidgetTypesArray.Add(MakeShareable(new FJsonValueObject(WidgetInfo)));
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("widget_types"), WidgetTypesArray);
        Result->SetNumberField(TEXT("count"), WidgetTypesArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAvailableWidgetTypesHandler)
