#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"

class FMcpMoveAnimNodeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("move_anim_node"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NodeId;
        if (!Params->TryGetStringField(TEXT("node_id"), NodeId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'node_id' parameter"), TEXT("MISSING_PARAM"));
        }

        float NewPosX = 0.0f;
        float NewPosY = 0.0f;

        if (Params->HasField(TEXT("position")))
        {
            TSharedPtr<FJsonObject> PositionObj = Params->GetObjectField(TEXT("position"));
            NewPosX = PositionObj->GetNumberField(TEXT("x"));
            NewPosY = PositionObj->GetNumberField(TEXT("y"));
        }
        else
        {
            if (!Params->TryGetNumberField(TEXT("pos_x"), NewPosX))
            {
                return FMcpCommandResult::Failure(TEXT("Missing 'position' or 'pos_x' parameter"), TEXT("MISSING_PARAM"));
            }
            if (!Params->TryGetNumberField(TEXT("pos_y"), NewPosY))
            {
                return FMcpCommandResult::Failure(TEXT("Missing 'position' or 'pos_y' parameter"), TEXT("MISSING_PARAM"));
            }
        }

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        FGuid NodeGuid;
        if (!FGuid::Parse(NodeId, NodeGuid))
        {
            return FMcpCommandResult::Failure(TEXT("Invalid node_id format"), TEXT("INVALID_GUID"));
        }

        UEdGraphNode* TargetNode = nullptr;

        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(AnimBP->UbergraphPages);
        AllGraphs.Append(AnimBP->FunctionGraphs);
        AllGraphs.Append(AnimBP->MacroGraphs);
        AllGraphs.Append(AnimBP->AnimationLayerGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (Node && Node->NodeGuid == NodeGuid)
                {
                    TargetNode = Node;
                    break;
                }
            }

            if (TargetNode) break;
        }

        if (!TargetNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Node not found: %s"), *NodeId),
                TEXT("NODE_NOT_FOUND")
            );
        }

        float OldPosX = TargetNode->NodePosX;
        float OldPosY = TargetNode->NodePosY;

        TargetNode->NodePosX = NewPosX;
        TargetNode->NodePosY = NewPosY;

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("moved"), true);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("node_name"), TargetNode->GetName());
        Result->SetNumberField(TEXT("old_pos_x"), OldPosX);
        Result->SetNumberField(TEXT("old_pos_y"), OldPosY);
        Result->SetNumberField(TEXT("new_pos_x"), NewPosX);
        Result->SetNumberField(TEXT("new_pos_y"), NewPosY);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpMoveAnimNodeHandler)