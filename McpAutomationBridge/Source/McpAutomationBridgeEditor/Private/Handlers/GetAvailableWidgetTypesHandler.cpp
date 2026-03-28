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
#include "AssetRegistry/AssetRegistryModule.h"

class FMcpGetAvailableWidgetTypesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_available_widget_types"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Category;
        Params->TryGetStringField(TEXT("category"), Category);

        TArray<TSharedPtr<FJsonValue>> WidgetTypesArray;

        TArray<TPair<FString, TArray<FString>>> WidgetCategories;

        WidgetCategories.Add(TPair<FString, TArray<FString>>(TEXT("Basic"), {
            TEXT("TextBlock"),
            TEXT("Image"),
            TEXT("Button"),
            TEXT("CheckBox"),
            TEXT("ProgressBar"),
            TEXT("Slider"),
            TEXT("SpinBox"),
            TEXT("Border"),
            TEXT("Spacer"),
        }));

        WidgetCategories.Add(TPair<FString, TArray<FString>>(TEXT("Input"), {
            TEXT("EditableText"),
            TEXT("EditableTextBox"),
            TEXT("MultiLineEditableText"),
            TEXT("MultiLineEditableTextBox"),
            TEXT("ComboBoxString"),
            TEXT("NumericUpDown"),
        }));

        WidgetCategories.Add(TPair<FString, TArray<FString>>(TEXT("Panels"), {
            TEXT("CanvasPanel"),
            TEXT("HorizontalBox"),
            TEXT("VerticalBox"),
            TEXT("GridPanel"),
            TEXT("UniformGridPanel"),
            TEXT("Overlay"),
            TEXT("ScrollBox"),
            TEXT("SizeBox"),
            TEXT("ScaleBox"),
            TEXT("WrapBox"),
            TEXT("InvalidationBox"),
            TEXT("RetainerBox"),
            TEXT("SafeZone"),
        }));

        WidgetCategories.Add(TPair<FString, TArray<FString>>(TEXT("Lists"), {
            TEXT("ListView"),
            TEXT("TileView"),
            TEXT("TreeView"),
        }));

        WidgetCategories.Add(TPair<FString, TArray<FString>>(TEXT("Advanced"), {
            TEXT("RichTextBlock"),
            TEXT("Throbber"),
            TEXT("CircularThrobber"),
            TEXT("ExpandableArea"),
        }));

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
        GetDerivedClasses(UWidget::StaticClass(), AllWidgetClasses);

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

private:
    void GetDerivedClasses(UClass* BaseClass, TArray<UClass*>& DerivedClasses)
    {
        if (!BaseClass)
        {
            return;
        }

        TArray<UClass*> AllClasses;
        GetDerivedClassesRecursive(BaseClass, AllClasses);

        for (UClass* Class : AllClasses)
        {
            if (Class && Class->IsChildOf(BaseClass) && !Class->HasAnyClassFlags(CLASS_Abstract))
            {
                DerivedClasses.AddUnique(Class);
            }
        }
    }

    void GetDerivedClassesRecursive(UClass* BaseClass, TArray<UClass*>& OutClasses)
    {
        if (!BaseClass)
        {
            return;
        }

        TArray<UClass*> Children;
        BaseClass->GetChildren(Children);

        for (UClass* Child : Children)
        {
            if (Child)
            {
                OutClasses.Add(Child);
                GetDerivedClassesRecursive(Child, OutClasses);
            }
        }
    }
};

REGISTER_MCP_COMMAND(FMcpGetAvailableWidgetTypesHandler)
