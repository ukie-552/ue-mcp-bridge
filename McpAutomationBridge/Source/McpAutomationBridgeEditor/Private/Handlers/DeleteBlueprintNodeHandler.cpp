#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpDeleteBlueprintNodeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("delete_blueprint_node"); }

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

        bool bDeleteConnections = true;
        Params->TryGetBoolField(TEXT("delete_connections"), bDeleteConnections);

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

        UEdGraphNode* NodeToDelete = nullptr;
        UEdGraph* OwnerGraph = nullptr;

        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(Blueprint->UbergraphPages);
        AllGraphs.Append(Blueprint->FunctionGraphs);
        AllGraphs.Append(Blueprint->MacroGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;
            NodeToDelete = FMcpBlueprintUtils::FindNodeByGuid(Graph, NodeGuid);
            if (NodeToDelete)
            {
                OwnerGraph = Graph;
                break;
            }
        }

        if (!NodeToDelete)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Node not found: %s"), *NodeId),
                TEXT("NODE_NOT_FOUND")
            );
        }

        if (bDeleteConnections)
        {
            NodeToDelete->BreakAllNodeLinks();
        }

        OwnerGraph->RemoveNode(NodeToDelete);
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("deleted"), true);
        Result->SetStringField(TEXT("node_id"), NodeId);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpDeleteBlueprintNodeHandler)
