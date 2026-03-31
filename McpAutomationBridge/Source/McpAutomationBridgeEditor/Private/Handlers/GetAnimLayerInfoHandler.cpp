#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_LinkedAnimLayer.h"
#include "AnimGraphNode_LinkedAnimGraph.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"

class FMcpGetAnimLayerInfoHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_anim_layer_info"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> LinkedLayersArray;

        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(AnimBP->UbergraphPages);
        AllGraphs.Append(AnimBP->FunctionGraphs);
        AllGraphs.Append(AnimBP->MacroGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (!Node) continue;

                if (UAnimGraphNode_LinkedAnimLayer* LayerNode = Cast<UAnimGraphNode_LinkedAnimLayer>(Node))
                {
                    TSharedPtr<FJsonObject> LayerInfo = MakeShareable(new FJsonObject);
                    LayerInfo->SetStringField(TEXT("node_id"), LayerNode->NodeGuid.ToString());
                    LayerInfo->SetStringField(TEXT("node_name"), LayerNode->GetName());
                    LayerInfo->SetStringField(TEXT("node_type"), TEXT("LinkedAnimLayer"));
                    LayerInfo->SetStringField(TEXT("interface"), LayerNode->Node.Interface ? LayerNode->Node.Interface->GetName() : TEXT(""));
                    LayerInfo->SetStringField(TEXT("layer_name"), LayerNode->Node.Layer.ToString());
                    LayerInfo->SetNumberField(TEXT("pos_x"), LayerNode->NodePosX);
                    LayerInfo->SetNumberField(TEXT("pos_y"), LayerNode->NodePosY);

                    LinkedLayersArray.Add(MakeShareable(new FJsonValueObject(LayerInfo)));
                }
                else if (UAnimGraphNode_LinkedAnimGraph* LinkedGraphNode = Cast<UAnimGraphNode_LinkedAnimGraph>(Node))
                {
                    TSharedPtr<FJsonObject> GraphInfo = MakeShareable(new FJsonObject);
                    GraphInfo->SetStringField(TEXT("node_id"), LinkedGraphNode->NodeGuid.ToString());
                    GraphInfo->SetStringField(TEXT("node_name"), LinkedGraphNode->GetName());
                    GraphInfo->SetStringField(TEXT("node_type"), TEXT("LinkedAnimGraph"));
                    GraphInfo->SetStringField(TEXT("graph_name"), LinkedGraphNode->Node.AnimGraph ? LinkedGraphNode->Node.AnimGraph->GetName() : TEXT(""));
                    GraphInfo->SetNumberField(TEXT("pos_x"), LinkedGraphNode->NodePosX);
                    GraphInfo->SetNumberField(TEXT("pos_y"), LinkedGraphNode->NodePosY);

                    LinkedLayersArray.Add(MakeShareable(new FJsonValueObject(GraphInfo)));
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetArrayField(TEXT("linked_layers"), LinkedLayersArray);
        Result->SetNumberField(TEXT("layer_count"), LinkedLayersArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAnimLayerInfoHandler)