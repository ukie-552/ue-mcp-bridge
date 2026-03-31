#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

class FMcpGetAnimLayerGraphTreeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_anim_layer_graph_tree"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bIncludePins = true;
        Params->TryGetBoolField(TEXT("include_pins"), bIncludePins);

        bool bIncludeConnections = true;
        Params->TryGetBoolField(TEXT("include_connections"), bIncludeConnections);

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> GraphsArray;
        int32 TotalNodes = 0;
        int32 TotalConnections = 0;

        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(AnimBP->UbergraphPages);
        AllGraphs.Append(AnimBP->FunctionGraphs);
        AllGraphs.Append(AnimBP->MacroGraphs);
        AllGraphs.Append(AnimBP->AnimationLayerGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;

            TSharedPtr<FJsonObject> GraphInfo = MakeShareable(new FJsonObject);
            FString GraphName = Graph->GetName();
            GraphInfo->SetStringField(TEXT("graph_name"), GraphName);

            bool bIsUbergraph = GraphName.Contains(TEXT("EventGraph"));
            bool bIsFunction = AnimBP->FunctionGraphs.Contains(Graph);
            bool bIsMacro = AnimBP->MacroGraphs.Contains(Graph);
            bool bIsLayer = AnimBP->AnimationLayerGraphs.Contains(Graph);

            GraphInfo->SetBoolField(TEXT("is_ubergraph"), bIsUbergraph);
            GraphInfo->SetBoolField(TEXT("is_function"), bIsFunction);
            GraphInfo->SetBoolField(TEXT("is_macro"), bIsMacro);
            GraphInfo->SetBoolField(TEXT("is_animation_layer"), bIsLayer);

            TArray<TSharedPtr<FJsonValue>> NodesArray;
            int32 GraphNodeCount = 0;

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (!Node) continue;

                GraphNodeCount++;

                TSharedPtr<FJsonObject> NodeInfo = MakeShareable(new FJsonObject);
                NodeInfo->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
                NodeInfo->SetStringField(TEXT("node_name"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
                NodeInfo->SetStringField(TEXT("node_type"), Node->GetClass()->GetName());

                TSharedPtr<FJsonObject> Position = MakeShareable(new FJsonObject);
                Position->SetNumberField(TEXT("x"), Node->NodePosX);
                Position->SetNumberField(TEXT("y"), Node->NodePosY);
                NodeInfo->SetObjectField(TEXT("position"), Position);

                FString Comment = Node->NodeComment.ToString();
                if (!Comment.IsEmpty())
                {
                    NodeInfo->SetStringField(TEXT("comment"), Comment);
                }

                if (bIncludePins)
                {
                    TArray<TSharedPtr<FJsonValue>> PinsArray;
                    int32 ConnectionCount = 0;

                    for (UEdGraphPin* Pin : Node->Pins)
                    {
                        if (!Pin) continue;

                        TSharedPtr<FJsonObject> PinInfo = MakeShareable(new FJsonObject);
                        PinInfo->SetStringField(TEXT("pin_id"), Pin->PinId.ToString());
                        PinInfo->SetStringField(TEXT("pin_name"), Pin->PinName);
                        PinInfo->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));
                        PinInfo->SetStringField(TEXT("pin_category"), Pin->PinType.PinCategory.IsNone() ? TEXT("None") : Pin->PinType.PinCategory.ToString());

                        ConnectionCount += Pin->LinkedTo.Num();

                        if (Pin->LinkedTo.Num() > 0)
                        {
                            TArray<TSharedPtr<FJsonValue>> LinkedPinsArray;
                            for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
                            {
                                if (LinkedPin && LinkedPin->GetOwningNode())
                                {
                                    TSharedPtr<FJsonObject> LinkedInfo = MakeShareable(new FJsonObject);
                                    LinkedInfo->SetStringField(TEXT("node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
                                    LinkedInfo->SetStringField(TEXT("pin_id"), LinkedPin->PinId.ToString());
                                    LinkedInfo->SetStringField(TEXT("pin_name"), LinkedPin->PinName);
                                    LinkedPinsArray.Add(MakeShareable(new FJsonValueObject(LinkedInfo)));
                                }
                            }
                            PinInfo->SetArrayField(TEXT("linked_to"), LinkedPinsArray);
                        }

                        PinsArray.Add(MakeShareable(new FJsonValueObject(PinInfo)));
                    }

                    NodeInfo->SetArrayField(TEXT("pins"), PinsArray);
                    NodeInfo->SetNumberField(TEXT("pin_count"), PinsArray.Num());
                    NodeInfo->SetNumberField(TEXT("connection_count"), ConnectionCount);
                    TotalConnections += ConnectionCount;
                }

                NodesArray.Add(MakeShareable(new FJsonValueObject(NodeInfo)));
            }

            GraphInfo->SetArrayField(TEXT("nodes"), NodesArray);
            GraphInfo->SetNumberField(TEXT("node_count"), GraphNodeCount);
            TotalNodes += GraphNodeCount;

            if (bIncludeConnections)
            {
                TArray<TSharedPtr<FJsonValue>> ConnectionsArray;

                for (UEdGraphNode* Node : Graph->Nodes)
                {
                    if (!Node) continue;

                    for (UEdGraphPin* Pin : Node->Pins)
                    {
                        if (!Pin || Pin->Direction != EGPD_Output) continue;

                        for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
                        {
                            if (!LinkedPin || !LinkedPin->GetOwningNode()) continue;

                            TSharedPtr<FJsonObject> Connection = MakeShareable(new FJsonObject);
                            Connection->SetStringField(TEXT("source_node_id"), Node->NodeGuid.ToString());
                            Connection->SetStringField(TEXT("source_pin_name"), Pin->PinName);
                            Connection->SetStringField(TEXT("target_node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
                            Connection->SetStringField(TEXT("target_pin_name"), LinkedPin->PinName);
                            ConnectionsArray.Add(MakeShareable(new FJsonValueObject(Connection)));
                        }
                    }
                }

                GraphInfo->SetArrayField(TEXT("connections"), ConnectionsArray);
                GraphInfo->SetNumberField(TEXT("connection_count"), ConnectionsArray.Num());
            }

            GraphsArray.Add(MakeShareable(new FJsonValueObject(GraphInfo)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetStringField(TEXT("anim_blueprint_name"), AnimBP->GetName());
        Result->SetArrayField(TEXT("graphs"), GraphsArray);
        Result->SetNumberField(TEXT("graph_count"), GraphsArray.Num());

        TSharedPtr<FJsonObject> Summary = MakeShareable(new FJsonObject);
        Summary->SetNumberField(TEXT("total_nodes"), TotalNodes);
        Summary->SetNumberField(TEXT("total_connections"), TotalConnections / 2);
        Summary->SetNumberField(TEXT("graphs_analyzed"), GraphsArray.Num());
        Result->SetObjectField(TEXT("summary"), Summary);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAnimLayerGraphTreeHandler)