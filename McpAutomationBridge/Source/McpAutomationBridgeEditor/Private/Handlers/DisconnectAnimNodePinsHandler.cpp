#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

class FMcpDisconnectAnimNodePinsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("disconnect_anim_node_pins"); }

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

        FString PinName;
        Params->TryGetStringField(TEXT("pin_name"), PinName);

        FString TargetNodeId;
        Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId);

        FString TargetPinName;
        Params->TryGetStringField(TEXT("target_pin_name"), TargetPinName);

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

        UEdGraphNode* SourceNode = nullptr;
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
                    SourceNode = Node;
                    FoundGraph = Graph;
                    break;
                }
            }

            if (SourceNode) break;
        }

        if (!SourceNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Node not found: %s"), *NodeId),
                TEXT("NODE_NOT_FOUND")
            );
        }

        int32 DisconnectedCount = 0;

        if (PinName.IsEmpty())
        {
            FoundGraph->Modify();

            for (UEdGraphPin* Pin : SourceNode->Pins)
            {
                if (!Pin) continue;

                for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
                {
                    if (LinkedPin)
                    {
                        LinkedPin->LinkedTo.Remove(SourceNode);
                        DisconnectedCount++;
                    }
                }
                Pin->LinkedTo.Empty();
                DisconnectedCount++;
            }
        }
        else
        {
            UEdGraphPin* SourcePin = nullptr;
            for (UEdGraphPin* Pin : SourceNode->Pins)
            {
                if (Pin && Pin->PinName == PinName)
                {
                    SourcePin = Pin;
                    break;
                }
            }

            if (!SourcePin)
            {
                return FMcpCommandResult::Failure(
                    FString::Printf(TEXT("Pin not found: %s"), *PinName),
                    TEXT("PIN_NOT_FOUND")
                );
            }

            FoundGraph->Modify();

            if (!TargetNodeId.IsEmpty() && !TargetPinName.IsEmpty())
            {
                FGuid TargetGuid;
                if (!FGuid::Parse(TargetNodeId, TargetGuid))
                {
                    return FMcpCommandResult::Failure(TEXT("Invalid target_node_id format"), TEXT("INVALID_GUID"));
                }

                for (UEdGraph* Graph : AllGraphs)
                {
                    if (!Graph) continue;

                    for (UEdGraphNode* Node : Graph->Nodes)
                    {
                        if (Node && Node->NodeGuid == TargetGuid)
                        {
                            for (UEdGraphPin* Pin : Node->Pins)
                            {
                                if (Pin && Pin->PinName == TargetPinName)
                                {
                                    SourcePin->LinkedTo.Remove(Pin);
                                    Pin->LinkedTo.Remove(SourcePin);
                                    DisconnectedCount = 1;
                                    break;
                                }
                            }
                            break;
                        }
                    }

                    if (DisconnectedCount > 0) break;
                }
            }
            else
            {
                TArray<UEdGraphPin*> LinkedPinsToRemove;
                for (UEdGraphPin* LinkedPin : SourcePin->LinkedTo)
                {
                    if (LinkedPin)
                    {
                        LinkedPin->LinkedTo.Remove(SourcePin);
                        LinkedPinsToRemove.Add(LinkedPin);
                        DisconnectedCount++;
                    }
                }

                for (UEdGraphPin* LinkedPin : LinkedPinsToRemove)
                {
                    SourcePin->LinkedTo.Remove(LinkedPin);
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("disconnected"), true);
        Result->SetStringField(TEXT("node_id"), NodeId);
        if (!PinName.IsEmpty())
        {
            Result->SetStringField(TEXT("pin_name"), PinName);
        }
        if (!TargetNodeId.IsEmpty())
        {
            Result->SetStringField(TEXT("target_node_id"), TargetNodeId);
        }
        Result->SetNumberField(TEXT("disconnected_count"), DisconnectedCount);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpDisconnectAnimNodePinsHandler)