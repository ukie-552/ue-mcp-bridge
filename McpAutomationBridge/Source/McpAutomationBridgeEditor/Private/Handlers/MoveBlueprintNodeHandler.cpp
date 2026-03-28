#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpMoveBlueprintNodeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("move_blueprint_node"); }

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

        const TSharedPtr<FJsonObject>* PositionObj;
        if (!Params->TryGetObjectField(TEXT("position"), PositionObj))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'position' parameter"), TEXT("MISSING_PARAM"));
        }

        int32 NewX = 0, NewY = 0;
        (*PositionObj)->TryGetNumberField(TEXT("x"), NewX);
        (*PositionObj)->TryGetNumberField(TEXT("y"), NewY);

        bool bAlignToGrid = true;
        Params->TryGetBoolField(TEXT("align_to_grid"), bAlignToGrid);

        if (bAlignToGrid)
        {
            NewX = FMath::RoundToInt(static_cast<float>(NewX) / 16.0f) * 16;
            NewY = FMath::RoundToInt(static_cast<float>(NewY) / 16.0f) * 16;
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

        UEdGraphNode* NodeToMove = nullptr;

        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(Blueprint->UbergraphPages);
        AllGraphs.Append(Blueprint->FunctionGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;
            NodeToMove = FMcpBlueprintUtils::FindNodeByGuid(Graph, NodeGuid);
            if (NodeToMove) break;
        }

        if (!NodeToMove)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Node not found: %s"), *NodeId),
                TEXT("NODE_NOT_FOUND")
            );
        }

        NodeToMove->Modify();
        NodeToMove->NodePosX = NewX;
        NodeToMove->NodePosY = NewY;

        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("node_id"), NodeId);
        
        TSharedPtr<FJsonObject> NewPosition = MakeShareable(new FJsonObject);
        NewPosition->SetNumberField(TEXT("x"), NewX);
        NewPosition->SetNumberField(TEXT("y"), NewY);
        Result->SetObjectField(TEXT("position"), NewPosition);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpMoveBlueprintNodeHandler)
