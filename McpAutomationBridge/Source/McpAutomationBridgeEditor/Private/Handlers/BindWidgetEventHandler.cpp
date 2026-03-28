#include "CoreMinimal.h"
#include "McpCommand.h"
#include "WidgetBlueprint.h"
#include "Components/Widget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/EditableText.h"
#include "Components/ComboBoxString.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_ComponentBoundEvent.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintActionDatabase.h"

class FMcpBindWidgetEventHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("bind_widget_event"); }

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

        FString EventType;
        if (!Params->TryGetStringField(TEXT("event_type"), EventType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'event_type' parameter"), TEXT("MISSING_PARAM"));
        }

        FString EventName;
        Params->TryGetStringField(TEXT("event_name"), EventName);

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

        UEdGraph* EventGraph = FindOrAddEventGraph(WidgetBP);
        if (!EventGraph)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to find or create event graph"), TEXT("NO_EVENT_GRAPH"));
        }

        UK2Node_ComponentBoundEvent* EventNode = CreateComponentBoundEvent(WidgetBP, Control, EventType, EventName, EventGraph);
        if (!EventNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create event binding for: %s"), *EventType),
                TEXT("EVENT_BINDING_FAILED")
            );
        }

        FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("bound"), true);
        Result->SetStringField(TEXT("control_name"), ControlName);
        Result->SetStringField(TEXT("event_type"), EventType);
        Result->SetStringField(TEXT("event_name"), EventNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
        Result->SetStringField(TEXT("widget_path"), WidgetPath);

        TSharedPtr<FJsonObject> NodeInfo = MakeShareable(new FJsonObject);
        NodeInfo->SetStringField(TEXT("node_id"), EventNode->NodeGuid.ToString());
        NodeInfo->SetNumberField(TEXT("pos_x"), EventNode->NodePosX);
        NodeInfo->SetNumberField(TEXT("pos_y"), EventNode->NodePosY);
        Result->SetObjectField(TEXT("node"), NodeInfo);

        return FMcpCommandResult::Success(Result);
    }

private:
    UEdGraph* FindOrAddEventGraph(UWidgetBlueprint* WidgetBP)
    {
        if (!WidgetBP)
        {
            return nullptr;
        }

        for (UEdGraph* Graph : WidgetBP->UbergraphPages)
        {
            if (Graph && Graph->GetName().Contains(TEXT("EventGraph")))
            {
                return Graph;
            }
        }

        if (WidgetBP->UbergraphPages.Num() > 0)
        {
            return WidgetBP->UbergraphPages[0];
        }

        return nullptr;
    }

    UK2Node_ComponentBoundEvent* CreateComponentBoundEvent(
        UWidgetBlueprint* WidgetBP,
        UWidget* Control,
        const FString& EventType,
        const FString& CustomEventName,
        UEdGraph* Graph)
    {
        if (!WidgetBP || !Control || !Graph)
        {
            return nullptr;
        }

        UClass* WidgetClass = Control->GetClass();
        FName EventFunctionName = GetEventFunctionName(EventType, WidgetClass);

        if (EventFunctionName.IsNone())
        {
            return nullptr;
        }

        UFunction* EventFunction = WidgetClass->FindFunctionByName(EventFunctionName);
        if (!EventFunction)
        {
            return nullptr;
        }

        UK2Node_ComponentBoundEvent* NewEventNode = NewObject<UK2Node_ComponentBoundEvent>(Graph);
        NewEventNode->SetFlags(RF_Transactional);
        Graph->AddNode(NewEventNode, false, false);

        NewEventNode->EventReference.SetExternalMember(EventFunctionName, WidgetClass);

        FName ComponentPropertyName = GetControlPropertyName(WidgetBP, Control);
        NewEventNode->ComponentPropertyName = ComponentPropertyName;

        if (!CustomEventName.IsEmpty())
        {
            NewEventNode->CustomFunctionName = *CustomEventName;
        }
        else
        {
            NewEventNode->CustomFunctionName = *FString::Printf(TEXT("On%s_%s"), *EventType, *Control->GetName());
        }

        NewEventNode->CreateNewGuid();
        NewEventNode->PostPlacedNewNode();
        NewEventNode->AllocateDefaultPins();

        int32 NodePosY = 0;
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (Node && Node->NodePosY > NodePosY)
            {
                NodePosY = Node->NodePosY;
            }
        }
        NewEventNode->NodePosY = NodePosY + 200;

        Graph->NotifyGraphChanged();

        return NewEventNode;
    }

    FName GetEventFunctionName(const FString& EventType, UClass* WidgetClass)
    {
        static TMap<FString, FName> CommonEvents = {
            {TEXT("OnClicked"), FName(TEXT("OnClicked"))},
            {TEXT("OnPressed"), FName(TEXT("OnPressed"))},
            {TEXT("OnReleased"), FName(TEXT("OnReleased"))},
            {TEXT("OnHovered"), FName(TEXT("OnHovered"))},
            {TEXT("OnUnhovered"), FName(TEXT("OnUnhovered"))},
            {TEXT("OnCheckStateChanged"), FName(TEXT("OnCheckStateChanged"))},
            {TEXT("OnValueChanged"), FName(TEXT("OnValueChanged"))},
            {TEXT("OnTextChanged"), FName(TEXT("OnTextChanged"))},
            {TEXT("OnTextCommitted"), FName(TEXT("OnTextCommitted"))},
            {TEXT("OnSelectionChanged"), FName(TEXT("OnSelectionChanged"))},
            {TEXT("OnMouseButtonDown"), FName(TEXT("OnMouseButtonDown"))},
            {TEXT("OnMouseButtonUp"), FName(TEXT("OnMouseButtonUp"))},
            {TEXT("OnMouseMove"), FName(TEXT("OnMouseMove"))},
            {TEXT("OnMouseEnter"), FName(TEXT("OnMouseEnter"))},
            {TEXT("OnMouseLeave"), FName(TEXT("OnMouseLeave"))},
            {TEXT("OnTouchStarted"), FName(TEXT("OnTouchStarted"))},
            {TEXT("OnTouchEnded"), FName(TEXT("OnTouchEnded"))},
        };

        FName* FoundEvent = CommonEvents.Find(EventType);
        if (FoundEvent)
        {
            return *FoundEvent;
        }

        return FName(*EventType);
    }

    FName GetControlPropertyName(UWidgetBlueprint* WidgetBP, UWidget* Control)
    {
        if (!WidgetBP || !Control)
        {
            return NAME_None;
        }

        UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
        if (!WidgetTree)
        {
            return NAME_None;
        }

        TArray<UWidget*> AllWidgets;
        WidgetTree->GetAllWidgets(AllWidgets);

        for (UWidget* Widget : AllWidgets)
        {
            if (Widget == Control)
            {
                return *Widget->GetName();
            }
        }

        return NAME_None;
    }
};

REGISTER_MCP_COMMAND(FMcpBindWidgetEventHandler)
