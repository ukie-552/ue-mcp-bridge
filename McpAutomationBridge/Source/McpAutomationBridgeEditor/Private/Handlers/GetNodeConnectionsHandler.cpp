#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

class FMcpGetNodeConnectionsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_node_connections"); }

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

        FGuid NodeGuid;
        if (!FGuid::Parse(NodeId, NodeGuid))
        {
            return FMcpCommandResult::Failure(TEXT("Invalid node GUID format"), TEXT("INVALID_GUID"));
        }

        UEdGraphNode* TargetNode = nullptr;

        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(Blueprint->UbergraphPages);
        AllGraphs.Append(Blueprint->FunctionGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;
            TargetNode = FMcpBlueprintUtils::FindNodeByGuid(Graph, NodeGuid);
            if (TargetNode) break;
        }

        if (!TargetNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Node not found: %s"), *NodeId),
                TEXT("NODE_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> ConnectionsArray;

        for (UEdGraphPin* Pin : TargetNode->Pins)
        {
            if (!Pin) continue;

            for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
            {
                if (!LinkedPin) continue;

                UEdGraphNode* LinkedNode = LinkedPin->GetOwningNode();
                if (!LinkedNode) continue;

                TSharedPtr<FJsonObject> Connection = MakeShareable(new FJsonObject);
                
                Connection->SetStringField(TEXT("pin_name"), Pin->PinName.ToString());
                Connection->SetStringField(TEXT("pin_direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
                
                TSharedPtr<FJsonObject> LinkedInfo = MakeShareable(new FJsonObject);
                LinkedInfo->SetStringField(TEXT("node_id"), LinkedNode->NodeGuid.ToString());
                LinkedInfo->SetStringField(TEXT("node_name"), LinkedNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
                LinkedInfo->SetStringField(TEXT("pin_name"), LinkedPin->PinName.ToString());
                LinkedInfo->SetStringField(TEXT("pin_direction"), LinkedPin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
                
                Connection->SetObjectField(TEXT("linked_to"), LinkedInfo);
                ConnectionsArray.Add(MakeShareable(new FJsonValueObject(Connection)));
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetArrayField(TEXT("connections"), ConnectionsArray);
        Result->SetNumberField(TEXT("count"), ConnectionsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetNodeConnectionsHandler)
