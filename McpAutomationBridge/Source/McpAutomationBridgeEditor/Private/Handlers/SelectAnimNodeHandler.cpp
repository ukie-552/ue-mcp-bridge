#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimationBlueprintEditor.h"
#include "BlueprintEditor.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Editor.h"

class FMcpSelectAnimNodeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("select_anim_node"); }

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

        bool bAppend = false;
        Params->TryGetBoolField(TEXT("append"), bAppend);

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
        UEdGraph* FoundGraph = nullptr;

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
                    FoundGraph = Graph;
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

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("selected"), true);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("node_name"), TargetNode->GetName());
        Result->SetStringField(TEXT("node_display_name"), TargetNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
        Result->SetStringField(TEXT("node_type"), TargetNode->GetClass()->GetName());
        Result->SetStringField(TEXT("graph_name"), FoundGraph ? FoundGraph->GetName() : TEXT("Unknown"));

        TSharedPtr<FJsonObject> NodeInfo = MakeShareable(new FJsonObject);
        NodeInfo->SetStringField(TEXT("node_id"), NodeId);
        NodeInfo->SetStringField(TEXT("node_guid"), NodeGuid.ToString());
        NodeInfo->SetStringField(TEXT("node_name"), TargetNode->GetName());
        NodeInfo->SetStringField(TEXT("node_title"), TargetNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
        NodeInfo->SetStringField(TEXT("node_class"), TargetNode->GetClass()->GetName());
        NodeInfo->SetStringField(TEXT("graph_name"), FoundGraph ? FoundGraph->GetName() : TEXT("Unknown"));

        TSharedPtr<FJsonObject> Position = MakeShareable(new FJsonObject);
        Position->SetNumberField(TEXT("x"), TargetNode->NodePosX);
        Position->SetNumberField(TEXT("y"), TargetNode->NodePosY);
        NodeInfo->SetObjectField(TEXT("position"), Position);

        Result->SetObjectField(TEXT("node_info"), NodeInfo);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSelectAnimNodeHandler)