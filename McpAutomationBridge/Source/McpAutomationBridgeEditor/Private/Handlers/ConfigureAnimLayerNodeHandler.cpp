#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_LinkedAnimLayer.h"
#include "AnimGraphNode_LinkedAnimGraph.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"

class FMcpConfigureAnimLayerNodeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("configure_anim_layer_node"); }

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

        UAnimGraphNode_LinkedAnimLayer* LayerNode = nullptr;
        UAnimGraphNode_LinkedAnimGraph* LinkedGraphNode = nullptr;

        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(AnimBP->UbergraphPages);
        AllGraphs.Append(AnimBP->FunctionGraphs);
        AllGraphs.Append(AnimBP->MacroGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (!Node || Node->NodeGuid != NodeGuid) continue;

                LayerNode = Cast<UAnimGraphNode_LinkedAnimLayer>(Node);
                if (!LayerNode)
                {
                    LinkedGraphNode = Cast<UAnimGraphNode_LinkedAnimGraph>(Node);
                }
                break;
            }

            if (LayerNode || LinkedGraphNode) break;
        }

        if (!LayerNode && !LinkedGraphNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Linked layer node not found: %s"), *NodeId),
                TEXT("NODE_NOT_FOUND")
            );
        }

        bool bConfigured = false;
        FString ConfiguredType;

        if (LayerNode)
        {
            FString InterfacePath;
            if (Params->TryGetStringField(TEXT("interface_path"), InterfacePath))
            {
                UClass* InterfaceClass = LoadClass<UObject>(nullptr, *InterfacePath);
                if (InterfaceClass)
                {
                    LayerNode->Node.Interface = InterfaceClass;
                    bConfigured = true;
                }
            }

            FString LayerName;
            if (Params->TryGetStringField(TEXT("layer_name"), LayerName))
            {
                LayerNode->Node.Layer = FName(*LayerName);
                bConfigured = true;
            }

            ConfiguredType = TEXT("LinkedAnimLayer");
        }
        else if (LinkedGraphNode)
        {
            FString GraphReference;
            if (Params->TryGetStringField(TEXT("graph_reference"), GraphReference))
            {
                LinkedGraphNode->Node.AnimGraphPath = *GraphReference;
                bConfigured = true;
            }

            ConfiguredType = TEXT("LinkedAnimGraph");
        }

        if (!bConfigured)
        {
            return FMcpCommandResult::Failure(TEXT("No valid configuration provided"), TEXT("NO_CONFIG"));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("configured"), true);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("node_type"), ConfiguredType);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpConfigureAnimLayerNodeHandler)