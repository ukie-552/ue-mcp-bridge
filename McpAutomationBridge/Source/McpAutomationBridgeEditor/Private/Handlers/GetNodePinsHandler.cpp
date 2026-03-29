#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

class FMcpGetNodePinsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_node_pins"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NodeId;
        if (!Params->TryGetStringField(TEXT("node_id"), NodeId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'node_id' parameter"), TEXT("MISSING_PARAM"));
        }

        UBlueprint* Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UEdGraphNode* Node = FMcpBlueprintUtils::FindNodeByGuid(Blueprint, NodeId);
        if (!Node)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Node not found: %s"), *NodeId),
                TEXT("NODE_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> PinsArray;

        for (UEdGraphPin* Pin : Node->Pins)
        {
            TSharedPtr<FJsonObject> PinInfo = MakeShareable(new FJsonObject);
            PinInfo->SetStringField(TEXT("pin_name"), Pin->PinName.ToString());
            PinInfo->SetStringField(TEXT("pin_type"), GetPinTypeString(Pin->PinType.PinCategory));
            PinInfo->SetStringField(TEXT("direction"), GetDirectionString(Pin->Direction));
            PinInfo->SetBoolField(TEXT("is_array"), Pin->PinType.ContainerType == EPinContainerType::Array);
            PinInfo->SetBoolField(TEXT("has_connection"), Pin->LinkedTo.Num() > 0);
            PinInfo->SetNumberField(TEXT("connection_count"), Pin->LinkedTo.Num());

            FString DefaultValue = Pin->GetDefaultAsString();
            if (!DefaultValue.IsEmpty())
            {
                PinInfo->SetStringField(TEXT("default_value"), DefaultValue);
            }

            TArray<TSharedPtr<FJsonValue>> LinkedPins;
            for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
            {
                TSharedPtr<FJsonObject> LinkedInfo = MakeShareable(new FJsonObject);
                LinkedInfo->SetStringField(TEXT("node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
                LinkedInfo->SetStringField(TEXT("pin_name"), LinkedPin->PinName.ToString());
                LinkedPins.Add(MakeShareable(new FJsonValueObject(LinkedInfo)));
            }
            PinInfo->SetArrayField(TEXT("linked_pins"), LinkedPins);

            PinsArray.Add(MakeShareable(new FJsonValueObject(PinInfo)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("node_name"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
        Result->SetArrayField(TEXT("pins"), PinsArray);

        return FMcpCommandResult::Success(Result);
    }

private:
    FString GetPinTypeString(const FName& PinCategory)
    {
        FString TypeStr = PinCategory.ToString();
        
        if (TypeStr == TEXT("bool")) return TEXT("bool");
        if (TypeStr == TEXT("int")) return TEXT("int");
        if (TypeStr == TEXT("float")) return TEXT("float");
        if (TypeStr == TEXT("string")) return TEXT("string");
        if (TypeStr == TEXT("name")) return TEXT("name");
        if (TypeStr == TEXT("text")) return TEXT("text");
        if (TypeStr == TEXT("vector")) return TEXT("vector");
        if (TypeStr == TEXT("rotator")) return TEXT("rotator");
        if (TypeStr == TEXT("transform")) return TEXT("transform");
        if (TypeStr == TEXT("color")) return TEXT("color");
        if (TypeStr == TEXT("object")) return TEXT("object");
        if (TypeStr == TEXT("class")) return TEXT("class");
        if (TypeStr == TEXT("struct")) return TEXT("struct");
        if (TypeStr == TEXT("enum")) return TEXT("enum");
        if (TypeStr == TEXT("delegate")) return TEXT("delegate");
        if (TypeStr == TEXT("exec")) return TEXT("exec");
        if (TypeStr == TEXT("wildcard")) return TEXT("wildcard");
        
        return TypeStr;
    }

    FString GetDirectionString(EEdGraphPinDirection Direction)
    {
        switch (Direction)
        {
        case EGPD_Input:
            return TEXT("input");
        case EGPD_Output:
            return TEXT("output");
        default:
            return TEXT("unknown");
        }
    }
};

REGISTER_MCP_COMMAND(FMcpGetNodePinsHandler)
