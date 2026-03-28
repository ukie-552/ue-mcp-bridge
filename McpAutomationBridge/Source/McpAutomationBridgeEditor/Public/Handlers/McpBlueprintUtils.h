#pragma once

#include "CoreMinimal.h"
#include "McpCommand.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "BlueprintEditor.h"

class FMcpBlueprintUtils
{
public:
    static UBlueprint* LoadBlueprint(const FString& BlueprintPath)
    {
        UObject* Asset = LoadObject<UObject>(nullptr, *BlueprintPath);
        if (!Asset)
        {
            return nullptr;
        }
        
        UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
        if (!Blueprint)
        {
            Blueprint = Cast<UBlueprint>(Asset->GetClass()->ClassGeneratedBy);
        }
        
        return Blueprint;
    }

    static UEdGraph* GetGraphByName(UBlueprint* Blueprint, const FString& GraphName)
    {
        if (!Blueprint)
        {
            return nullptr;
        }

        if (GraphName.IsEmpty() || GraphName == TEXT("EventGraph"))
        {
            for (UEdGraph* Graph : Blueprint->UbergraphPages)
            {
                if (Graph && Graph->GetName().Contains(TEXT("EventGraph")))
                {
                    return Graph;
                }
            }
            if (Blueprint->UbergraphPages.Num() > 0)
            {
                return Blueprint->UbergraphPages[0];
            }
        }

        for (UEdGraph* Graph : Blueprint->UbergraphPages)
        {
            if (Graph && Graph->GetName() == GraphName)
            {
                return Graph;
            }
        }

        for (UEdGraph* Graph : Blueprint->FunctionGraphs)
        {
            if (Graph && Graph->GetName() == GraphName)
            {
                return Graph;
            }
        }

        return nullptr;
    }

    static TSharedPtr<FJsonObject> NodeToJson(UEdGraphNode* Node)
    {
        if (!Node)
        {
            return nullptr;
        }

        TSharedPtr<FJsonObject> NodeJson = MakeShareable(new FJsonObject);
        NodeJson->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
        NodeJson->SetStringField(TEXT("node_name"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
        NodeJson->SetStringField(TEXT("node_type"), Node->GetClass()->GetName());

        TSharedPtr<FJsonObject> Position = MakeShareable(new FJsonObject);
        Position->SetNumberField(TEXT("x"), Node->NodePosX);
        Position->SetNumberField(TEXT("y"), Node->NodePosY);
        NodeJson->SetObjectField(TEXT("position"), Position);

        TArray<TSharedPtr<FJsonValue>> PinsArray;
        for (UEdGraphPin* Pin : Node->Pins)
        {
            TSharedPtr<FJsonObject> PinJson = MakeShareable(new FJsonObject);
            PinJson->SetStringField(TEXT("pin_name"), Pin->PinName.ToString());
            PinJson->SetStringField(TEXT("pin_type"), Pin->PinType.PinCategory.ToString());
            PinJson->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
            PinJson->SetBoolField(TEXT("is_array"), Pin->PinType.IsArray());
            PinJson->SetBoolField(TEXT("has_connection"), Pin->LinkedTo.Num() > 0);
            PinsArray.Add(MakeShareable(new FJsonValueObject(PinJson)));
        }
        NodeJson->SetArrayField(TEXT("pins"), PinsArray);

        return NodeJson;
    }

    static UEdGraphNode* FindNodeByGuid(UEdGraph* Graph, const FGuid& NodeGuid)
    {
        if (!Graph)
        {
            return nullptr;
        }

        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (Node && Node->NodeGuid == NodeGuid)
            {
                return Node;
            }
        }

        return nullptr;
    }

    static UEdGraphNode* FindNodeByGuid(UBlueprint* Blueprint, const FString& NodeId)
    {
        if (!Blueprint)
        {
            return nullptr;
        }

        FGuid NodeGuid;
        if (!FGuid::Parse(NodeId, NodeGuid))
        {
            return nullptr;
        }

        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(Blueprint->UbergraphPages);
        AllGraphs.Append(Blueprint->FunctionGraphs);
        AllGraphs.Append(Blueprint->MacroGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph)
            {
                continue;
            }

            UEdGraphNode* FoundNode = FindNodeByGuid(Graph, NodeGuid);
            if (FoundNode)
            {
                return FoundNode;
            }
        }

        return nullptr;
    }

    static UEdGraphPin* FindPin(UEdGraphNode* Node, const FString& PinName, EEdGraphPinDirection Direction = EGPD_MAX)
    {
        if (!Node)
        {
            return nullptr;
        }

        for (UEdGraphPin* Pin : Node->Pins)
        {
            if (Pin && Pin->PinName.ToString() == PinName)
            {
                if (Direction == EGPD_MAX || Pin->Direction == Direction)
                {
                    return Pin;
                }
            }
        }

        return nullptr;
    }

    static UEdGraphPin* FindPin(UBlueprint* Blueprint, const FString& NodeId, const FString& PinName)
    {
        if (!Blueprint)
        {
            return nullptr;
        }

        UEdGraphNode* Node = FindNodeByGuid(Blueprint, NodeId);
        if (Node)
        {
            return FindPin(Node, PinName);
        }

        return nullptr;
    }
};
