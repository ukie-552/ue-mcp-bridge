#include "CoreMinimal.h"
#include "McpCommand.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Styling/SlateColor.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "UObject/PropertyPortFlags.h"

class FMcpSetWidgetPropertyHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_widget_property"); }

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

        FString PropertyName;
        if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'property_name' parameter"), TEXT("MISSING_PARAM"));
        }

        const TSharedPtr<FJsonObject>* PropertyValueObj;
        if (!Params->TryGetObjectField(TEXT("property_value"), PropertyValueObj))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'property_value' parameter"), TEXT("MISSING_PARAM"));
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

        bool bSuccess = SetProperty(Control, PropertyName, *PropertyValueObj);

        if (bSuccess)
        {
            FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("success"), bSuccess);
        Result->SetStringField(TEXT("control_name"), ControlName);
        Result->SetStringField(TEXT("property_name"), PropertyName);

        return FMcpCommandResult::Success(Result);
    }

private:
    bool SetProperty(UWidget* Widget, const FString& PropertyName, const TSharedPtr<FJsonObject>& ValueObj)
    {
        if (!Widget)
        {
            return false;
        }

        UClass* WidgetClass = Widget->GetClass();
        FProperty* Property = WidgetClass->FindPropertyByName(*PropertyName);

        if (!Property)
        {
            return false;
        }

        void* PropertyAddr = Property->ContainerPtrToValuePtr<void>(Widget);

        if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
        {
            FString Value;
            if (ValueObj->TryGetStringField(TEXT("value"), Value))
            {
                StrProp->SetPropertyValue(PropertyAddr, Value);
                return true;
            }
        }
        else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
        {
            FString Value;
            if (ValueObj->TryGetStringField(TEXT("value"), Value))
            {
                NameProp->SetPropertyValue(PropertyAddr, *Value);
                return true;
            }
        }
        else if (FTextProperty* TextProp = CastField<FTextProperty>(Property))
        {
            FString Value;
            if (ValueObj->TryGetStringField(TEXT("value"), Value))
            {
                TextProp->SetPropertyValue(PropertyAddr, FText::FromString(Value));
                return true;
            }
        }
        else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
        {
            bool Value;
            if (ValueObj->TryGetBoolField(TEXT("value"), Value))
            {
                BoolProp->SetPropertyValue(PropertyAddr, Value);
                return true;
            }
        }
        else if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
        {
            int32 Value;
            if (ValueObj->TryGetNumberField(TEXT("value"), Value))
            {
                IntProp->SetPropertyValue(PropertyAddr, Value);
                return true;
            }
        }
        else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
        {
            double Value;
            if (ValueObj->TryGetNumberField(TEXT("value"), Value))
            {
                FloatProp->SetPropertyValue(PropertyAddr, static_cast<float>(Value));
                return true;
            }
        }
        else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Property))
        {
            double Value;
            if (ValueObj->TryGetNumberField(TEXT("value"), Value))
            {
                DoubleProp->SetPropertyValue(PropertyAddr, Value);
                return true;
            }
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
        {
            return SetStructProperty(Widget, StructProp, PropertyAddr, ValueObj);
        }
        else if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Property))
        {
            return SetObjectProperty(Widget, ObjProp, PropertyAddr, ValueObj);
        }

        return false;
    }

    bool SetStructProperty(UWidget* Widget, FStructProperty* StructProp, void* PropertyAddr, const TSharedPtr<FJsonObject>& ValueObj)
    {
        UScriptStruct* Struct = StructProp->Struct;

        if (Struct == TBaseStructure<FVector2D>::Get())
        {
            FVector2D* Vec = static_cast<FVector2D*>(PropertyAddr);
            double X = 0, Y = 0;
            ValueObj->TryGetNumberField(TEXT("x"), X);
            ValueObj->TryGetNumberField(TEXT("y"), Y);
            Vec->X = static_cast<float>(X);
            Vec->Y = static_cast<float>(Y);
            return true;
        }
        else if (Struct == TBaseStructure<FLinearColor>::Get())
        {
            FLinearColor* Color = static_cast<FLinearColor*>(PropertyAddr);
            double R = 0, G = 0, B = 0, A = 1;
            ValueObj->TryGetNumberField(TEXT("r"), R);
            ValueObj->TryGetNumberField(TEXT("g"), G);
            ValueObj->TryGetNumberField(TEXT("b"), B);
            ValueObj->TryGetNumberField(TEXT("a"), A);
            *Color = FLinearColor(R, G, B, A);
            return true;
        }
        else if (Struct == TBaseStructure<FSlateColor>::Get())
        {
            double R = 0, G = 0, B = 0, A = 1;
            ValueObj->TryGetNumberField(TEXT("r"), R);
            ValueObj->TryGetNumberField(TEXT("g"), G);
            ValueObj->TryGetNumberField(TEXT("b"), B);
            ValueObj->TryGetNumberField(TEXT("a"), A);
            FSlateColor* SlateColor = static_cast<FSlateColor*>(PropertyAddr);
            *SlateColor = FSlateColor(FLinearColor(R, G, B, A));
            return true;
        }
        else if (Struct == TBaseStructure<FMargin>::Get())
        {
            FMargin* Margin = static_cast<FMargin*>(PropertyAddr);
            double Left = 0, Top = 0, Right = 0, Bottom = 0;
            ValueObj->TryGetNumberField(TEXT("left"), Left);
            ValueObj->TryGetNumberField(TEXT("top"), Top);
            ValueObj->TryGetNumberField(TEXT("right"), Right);
            ValueObj->TryGetNumberField(TEXT("bottom"), Bottom);
            *Margin = FMargin(Left, Top, Right, Bottom);
            return true;
        }
        else if (Struct == TBaseStructure<FVector>::Get())
        {
            FVector* Vec = static_cast<FVector*>(PropertyAddr);
            double X = 0, Y = 0, Z = 0;
            ValueObj->TryGetNumberField(TEXT("x"), X);
            ValueObj->TryGetNumberField(TEXT("y"), Y);
            ValueObj->TryGetNumberField(TEXT("z"), Z);
            Vec->X = static_cast<float>(X);
            Vec->Y = static_cast<float>(Y);
            Vec->Z = static_cast<float>(Z);
            return true;
        }
        else if (Struct == TBaseStructure<FRotator>::Get())
        {
            FRotator* Rot = static_cast<FRotator*>(PropertyAddr);
            double Pitch = 0, Yaw = 0, Roll = 0;
            ValueObj->TryGetNumberField(TEXT("pitch"), Pitch);
            ValueObj->TryGetNumberField(TEXT("yaw"), Yaw);
            ValueObj->TryGetNumberField(TEXT("roll"), Roll);
            *Rot = FRotator(Pitch, Yaw, Roll);
            return true;
        }

        return false;
    }

    bool SetObjectProperty(UWidget* Widget, FObjectProperty* ObjProp, void* PropertyAddr, const TSharedPtr<FJsonObject>& ValueObj)
    {
        FString AssetPath;
        if (ValueObj->TryGetStringField(TEXT("asset_path"), AssetPath))
        {
            UClass* PropertyClass = ObjProp->PropertyClass;
            UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);

            if (Asset && Asset->IsA(PropertyClass))
            {
                ObjProp->SetObjectPropertyValue(PropertyAddr, Asset);
                return true;
            }
        }

        return false;
    }
};

REGISTER_MCP_COMMAND(FMcpSetWidgetPropertyHandler)
