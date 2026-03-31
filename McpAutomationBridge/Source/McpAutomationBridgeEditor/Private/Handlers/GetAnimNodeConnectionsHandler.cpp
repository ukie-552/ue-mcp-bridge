#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

class FMcpGetAnimNodeConnectionsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_anim_node_connections"); }

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

                Connection->SetStringField(TEXT("from_pin_id"), Pin->PinId.ToString());
                Connection->SetStringField(TEXT("from_pin_name"), Pin->PinName);
                Connection->SetStringField(TEXT("from_direction"), Pin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));

                TSharedPtr<FJsonObject> TargetInfo = MakeShareable(new FJsonObject);
                TargetInfo->SetStringField(TEXT("node_id"), LinkedNode->NodeGuid.ToString());
                TargetInfo->SetStringField(TEXT("node_name"), LinkedNode->GetName());
                TargetInfo->SetStringField(TEXT("node_display_name"), LinkedNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
                TargetInfo->SetStringField(TEXT("node_type"), LinkedNode->GetClass()->GetName());
                TargetInfo->SetStringField(TEXT("pin_id"), LinkedPin->PinId.ToString());
                TargetInfo->SetStringField(TEXT("pin_name"), LinkedPin->PinName);
                TargetInfo->SetStringField(TEXT("direction"), LinkedPin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));

                Connection->SetObjectField(TEXT("to_node"), TargetInfo);
                ConnectionsArray.Add(MakeShareable(new FJsonValueObject(Connection)));
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("node_name"), TargetNode->GetName());
        Result->SetStringField(TEXT("node_display_name"), TargetNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
        Result->SetArrayField(TEXT("connections"), ConnectionsArray);
        Result->SetNumberField(TEXT("connection_count"), ConnectionsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAnimNodeConnectionsHandler)