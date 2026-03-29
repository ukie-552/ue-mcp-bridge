#include "CoreMinimal.h"
#include "McpCommand.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/BorderSlot.h"
#include "Components/UniformGridSlot.h"
#include "Components/WrapBoxSlot.h"
#include "Components/ScaleBoxSlot.h"
#include "Components/SizeBoxSlot.h"
#include "Components/GridSlot.h"
#include "Components/ScrollBoxSlot.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpSetWidgetLayoutHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_widget_layout"); }

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

        UPanelSlot* Slot = Control->Slot;
        if (!Slot)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Control has no slot: %s"), *ControlName),
                TEXT("NO_SLOT")
            );
        }

        bool bModified = false;

        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
        {
            bModified = ApplyCanvasSlotLayout(CanvasSlot, Params);
        }
        else if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Slot))
        {
            bModified = ApplyOverlaySlotLayout(OverlaySlot, Params);
        }
        else if (UVerticalBoxSlot* VerticalSlot = Cast<UVerticalBoxSlot>(Slot))
        {
            bModified = ApplyVerticalBoxSlotLayout(VerticalSlot, Params);
        }
        else if (UHorizontalBoxSlot* HorizontalSlot = Cast<UHorizontalBoxSlot>(Slot))
        {
            bModified = ApplyHorizontalBoxSlotLayout(HorizontalSlot, Params);
        }
        else if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(Slot))
        {
            bModified = ApplyBorderSlotLayout(BorderSlot, Params);
        }
        else if (UUniformGridSlot* UniformGridSlot = Cast<UUniformGridSlot>(Slot))
        {
            bModified = ApplyUniformGridSlotLayout(UniformGridSlot, Params);
        }
        else if (UWrapBoxSlot* WrapSlot = Cast<UWrapBoxSlot>(Slot))
        {
            bModified = ApplyWrapBoxSlotLayout(WrapSlot, Params);
        }
        else if (UScaleBoxSlot* ScaleSlot = Cast<UScaleBoxSlot>(Slot))
        {
            bModified = ApplyScaleBoxSlotLayout(ScaleSlot, Params);
        }
        else if (USizeBoxSlot* SizeSlot = Cast<USizeBoxSlot>(Slot))
        {
            bModified = ApplySizeBoxSlotLayout(SizeSlot, Params);
        }
        else if (UGridSlot* GridSlot = Cast<UGridSlot>(Slot))
        {
            bModified = ApplyGridSlotLayout(GridSlot, Params);
        }
        else if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(Slot))
        {
            bModified = ApplyScrollBoxSlotLayout(ScrollSlot, Params);
        }

        if (bModified)
        {
            FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("success"), bModified);
        Result->SetStringField(TEXT("control_name"), ControlName);
        Result->SetStringField(TEXT("slot_type"), Slot->GetClass()->GetName());

        return FMcpCommandResult::Success(Result);
    }

private:
    bool ApplyCanvasSlotLayout(UCanvasPanelSlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        const TSharedPtr<FJsonObject>* AnchorsObj;
        if (Params->TryGetObjectField(TEXT("anchors"), AnchorsObj))
        {
            FVector2D Min(0, 0), Max(0, 0);
            GetVector2DFromJson(*AnchorsObj, TEXT("minimum"), Min);
            GetVector2DFromJson(*AnchorsObj, TEXT("maximum"), Max);
            Slot->SetAnchors(FAnchors(Min.X, Min.Y, Max.X, Max.Y));
            bModified = true;
        }

        const TSharedPtr<FJsonObject>* PositionObj;
        if (Params->TryGetObjectField(TEXT("position"), PositionObj))
        {
            FVector2D Position;
            if (GetVector2DFromJson(*PositionObj, TEXT("value"), Position))
            {
                Slot->SetPosition(Position);
                bModified = true;
            }
        }

        const TSharedPtr<FJsonObject>* SizeObj;
        if (Params->TryGetObjectField(TEXT("size"), SizeObj))
        {
            FVector2D Size;
            if (GetVector2DFromJson(*SizeObj, TEXT("value"), Size))
            {
                Slot->SetSize(Size);
                bModified = true;
            }
        }

        const TSharedPtr<FJsonObject>* AlignmentObj;
        if (Params->TryGetObjectField(TEXT("alignment"), AlignmentObj))
        {
            FVector2D Alignment;
            if (GetVector2DFromJson(*AlignmentObj, TEXT("value"), Alignment))
            {
                Slot->SetAlignment(Alignment);
                bModified = true;
            }
        }

        const TSharedPtr<FJsonObject>* OffsetsObj;
        if (Params->TryGetObjectField(TEXT("offsets"), OffsetsObj))
        {
            FMargin Offsets;
            GetMarginFromJson(*OffsetsObj, Offsets);
            Slot->SetOffsets(Offsets);
            bModified = true;
        }

        int32 ZOrder;
        if (Params->TryGetNumberField(TEXT("zorder"), ZOrder))
        {
            Slot->SetZOrder(ZOrder);
            bModified = true;
        }

        return bModified;
    }

    bool ApplyOverlaySlotLayout(UOverlaySlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        const TSharedPtr<FJsonObject>* PaddingObj;
        if (Params->TryGetObjectField(TEXT("padding"), PaddingObj))
        {
            FMargin Padding;
            GetMarginFromJson(*PaddingObj, Padding);
            Slot->SetPadding(Padding);
            bModified = true;
        }

        FString HAlign, VAlign;
        if (Params->TryGetStringField(TEXT("h_align"), HAlign))
        {
            Slot->SetHorizontalAlignment(StringToHAlign(HAlign));
            bModified = true;
        }
        if (Params->TryGetStringField(TEXT("v_align"), VAlign))
        {
            Slot->SetVerticalAlignment(StringToVAlign(VAlign));
            bModified = true;
        }

        return bModified;
    }

    bool ApplyVerticalBoxSlotLayout(UVerticalBoxSlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        const TSharedPtr<FJsonObject>* PaddingObj;
        if (Params->TryGetObjectField(TEXT("padding"), PaddingObj))
        {
            FMargin Padding;
            GetMarginFromJson(*PaddingObj, Padding);
            Slot->SetPadding(Padding);
            bModified = true;
        }

        FString HAlign, VAlign;
        if (Params->TryGetStringField(TEXT("h_align"), HAlign))
        {
            Slot->SetHorizontalAlignment(StringToHAlign(HAlign));
            bModified = true;
        }
        if (Params->TryGetStringField(TEXT("v_align"), VAlign))
        {
            Slot->SetVerticalAlignment(StringToVAlign(VAlign));
            bModified = true;
        }

        double SizeValue;
        if (Params->TryGetNumberField(TEXT("size"), SizeValue))
        {
            FSlateChildSize ChildSize;
            ChildSize.Value = static_cast<float>(SizeValue);
            ChildSize.SizeRule = ESlateSizeRule::Fill;
            Slot->SetSize(ChildSize);
            bModified = true;
        }

        return bModified;
    }

    bool ApplyHorizontalBoxSlotLayout(UHorizontalBoxSlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        const TSharedPtr<FJsonObject>* PaddingObj;
        if (Params->TryGetObjectField(TEXT("padding"), PaddingObj))
        {
            FMargin Padding;
            GetMarginFromJson(*PaddingObj, Padding);
            Slot->SetPadding(Padding);
            bModified = true;
        }

        FString HAlign, VAlign;
        if (Params->TryGetStringField(TEXT("h_align"), HAlign))
        {
            Slot->SetHorizontalAlignment(StringToHAlign(HAlign));
            bModified = true;
        }
        if (Params->TryGetStringField(TEXT("v_align"), VAlign))
        {
            Slot->SetVerticalAlignment(StringToVAlign(VAlign));
            bModified = true;
        }

        double SizeValue;
        if (Params->TryGetNumberField(TEXT("size"), SizeValue))
        {
            FSlateChildSize ChildSize;
            ChildSize.Value = static_cast<float>(SizeValue);
            ChildSize.SizeRule = ESlateSizeRule::Fill;
            Slot->SetSize(ChildSize);
            bModified = true;
        }

        return bModified;
    }

    bool ApplyBorderSlotLayout(UBorderSlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        const TSharedPtr<FJsonObject>* PaddingObj;
        if (Params->TryGetObjectField(TEXT("padding"), PaddingObj))
        {
            FMargin Padding;
            GetMarginFromJson(*PaddingObj, Padding);
            Slot->SetPadding(Padding);
            bModified = true;
        }

        FString HAlign, VAlign;
        if (Params->TryGetStringField(TEXT("h_align"), HAlign))
        {
            Slot->SetHorizontalAlignment(StringToHAlign(HAlign));
            bModified = true;
        }
        if (Params->TryGetStringField(TEXT("v_align"), VAlign))
        {
            Slot->SetVerticalAlignment(StringToVAlign(VAlign));
            bModified = true;
        }

        return bModified;
    }

    bool ApplyUniformGridSlotLayout(UUniformGridSlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        int32 Row, Column;
        if (Params->TryGetNumberField(TEXT("row"), Row))
        {
            Slot->SetRow(Row);
            bModified = true;
        }
        if (Params->TryGetNumberField(TEXT("column"), Column))
        {
            Slot->SetColumn(Column);
            bModified = true;
        }

        return bModified;
    }

    bool ApplyWrapBoxSlotLayout(UWrapBoxSlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        const TSharedPtr<FJsonObject>* PaddingObj;
        if (Params->TryGetObjectField(TEXT("padding"), PaddingObj))
        {
            FMargin Padding;
            GetMarginFromJson(*PaddingObj, Padding);
            Slot->SetPadding(Padding);
            bModified = true;
        }

        FString HAlign, VAlign;
        if (Params->TryGetStringField(TEXT("h_align"), HAlign))
        {
            Slot->SetHorizontalAlignment(StringToHAlign(HAlign));
            bModified = true;
        }
        if (Params->TryGetStringField(TEXT("v_align"), VAlign))
        {
            Slot->SetVerticalAlignment(StringToVAlign(VAlign));
            bModified = true;
        }

        return bModified;
    }

    bool ApplyScaleBoxSlotLayout(UScaleBoxSlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        const TSharedPtr<FJsonObject>* PaddingObj;
        if (Params->TryGetObjectField(TEXT("padding"), PaddingObj))
        {
            FMargin Padding;
            GetMarginFromJson(*PaddingObj, Padding);
            Slot->SetPadding(Padding);
            bModified = true;
        }

        FString HAlign, VAlign;
        if (Params->TryGetStringField(TEXT("h_align"), HAlign))
        {
            Slot->SetHorizontalAlignment(StringToHAlign(HAlign));
            bModified = true;
        }
        if (Params->TryGetStringField(TEXT("v_align"), VAlign))
        {
            Slot->SetVerticalAlignment(StringToVAlign(VAlign));
            bModified = true;
        }

        return bModified;
    }

    bool ApplySizeBoxSlotLayout(USizeBoxSlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        const TSharedPtr<FJsonObject>* PaddingObj;
        if (Params->TryGetObjectField(TEXT("padding"), PaddingObj))
        {
            FMargin Padding;
            GetMarginFromJson(*PaddingObj, Padding);
            Slot->SetPadding(Padding);
            bModified = true;
        }

        FString HAlign, VAlign;
        if (Params->TryGetStringField(TEXT("h_align"), HAlign))
        {
            Slot->SetHorizontalAlignment(StringToHAlign(HAlign));
            bModified = true;
        }
        if (Params->TryGetStringField(TEXT("v_align"), VAlign))
        {
            Slot->SetVerticalAlignment(StringToVAlign(VAlign));
            bModified = true;
        }

        return bModified;
    }

    bool ApplyGridSlotLayout(UGridSlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        int32 Row, Column, RowSpan, ColumnSpan;
        if (Params->TryGetNumberField(TEXT("row"), Row))
        {
            Slot->SetRow(Row);
            bModified = true;
        }
        if (Params->TryGetNumberField(TEXT("column"), Column))
        {
            Slot->SetColumn(Column);
            bModified = true;
        }
        if (Params->TryGetNumberField(TEXT("row_span"), RowSpan))
        {
            Slot->SetRowSpan(RowSpan);
            bModified = true;
        }
        if (Params->TryGetNumberField(TEXT("column_span"), ColumnSpan))
        {
            Slot->SetColumnSpan(ColumnSpan);
            bModified = true;
        }

        const TSharedPtr<FJsonObject>* PaddingObj;
        if (Params->TryGetObjectField(TEXT("padding"), PaddingObj))
        {
            FMargin Padding;
            GetMarginFromJson(*PaddingObj, Padding);
            Slot->SetPadding(Padding);
            bModified = true;
        }

        return bModified;
    }

    bool ApplyScrollBoxSlotLayout(UScrollBoxSlot* Slot, const TSharedPtr<FJsonObject>& Params)
    {
        bool bModified = false;

        const TSharedPtr<FJsonObject>* PaddingObj;
        if (Params->TryGetObjectField(TEXT("padding"), PaddingObj))
        {
            FMargin Padding;
            GetMarginFromJson(*PaddingObj, Padding);
            Slot->SetPadding(Padding);
            bModified = true;
        }

        FString HAlign, VAlign;
        if (Params->TryGetStringField(TEXT("h_align"), HAlign))
        {
            Slot->SetHorizontalAlignment(StringToHAlign(HAlign));
            bModified = true;
        }
        if (Params->TryGetStringField(TEXT("v_align"), VAlign))
        {
            Slot->SetVerticalAlignment(StringToVAlign(VAlign));
            bModified = true;
        }

        return bModified;
    }

    bool GetVector2DFromJson(const TSharedPtr<FJsonObject>& JsonObj, const FString& FieldName, FVector2D& OutVector)
    {
        const TSharedPtr<FJsonObject>* VecObj;
        if (JsonObj->TryGetObjectField(FieldName, VecObj))
        {
            double X = 0, Y = 0;
            (*VecObj)->TryGetNumberField(TEXT("x"), X);
            (*VecObj)->TryGetNumberField(TEXT("y"), Y);
            OutVector = FVector2D(X, Y);
            return true;
        }
        return false;
    }

    void GetMarginFromJson(const TSharedPtr<FJsonObject>& JsonObj, FMargin& OutMargin)
    {
        double Left = 0, Top = 0, Right = 0, Bottom = 0;
        JsonObj->TryGetNumberField(TEXT("left"), Left);
        JsonObj->TryGetNumberField(TEXT("top"), Top);
        JsonObj->TryGetNumberField(TEXT("right"), Right);
        JsonObj->TryGetNumberField(TEXT("bottom"), Bottom);
        OutMargin = FMargin(Left, Top, Right, Bottom);
    }

    EHorizontalAlignment StringToHAlign(const FString& AlignStr)
    {
        if (AlignStr == TEXT("Left") || AlignStr == TEXT("left"))
            return HAlign_Left;
        if (AlignStr == TEXT("Center") || AlignStr == TEXT("center"))
            return HAlign_Center;
        if (AlignStr == TEXT("Right") || AlignStr == TEXT("right"))
            return HAlign_Right;
        if (AlignStr == TEXT("Fill") || AlignStr == TEXT("fill"))
            return HAlign_Fill;
        return HAlign_Left;
    }

    EVerticalAlignment StringToVAlign(const FString& AlignStr)
    {
        if (AlignStr == TEXT("Top") || AlignStr == TEXT("top"))
            return VAlign_Top;
        if (AlignStr == TEXT("Center") || AlignStr == TEXT("center"))
            return VAlign_Center;
        if (AlignStr == TEXT("Bottom") || AlignStr == TEXT("bottom"))
            return VAlign_Bottom;
        if (AlignStr == TEXT("Fill") || AlignStr == TEXT("fill"))
            return VAlign_Fill;
        return VAlign_Top;
    }
};

REGISTER_MCP_COMMAND(FMcpSetWidgetLayoutHandler)
