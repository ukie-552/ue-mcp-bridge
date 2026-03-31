#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_Base.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

class FMcpGetAnimNodePinsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_anim_node_pins"); }

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

        TArray<TSharedPtr<FJsonValue>> PinsArray;

        for (UEdGraphPin* Pin : TargetNode->Pins)
        {
            if (!Pin) continue;

            TSharedPtr<FJsonObject> PinInfo = MakeShareable(new FJsonObject);
            PinInfo->SetStringField(TEXT("pin_id"), Pin->PinId.ToString());
            PinInfo->SetStringField(TEXT("pin_name"), Pin->PinName);
            PinInfo->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));
            PinInfo->SetStringField(TEXT("pin_category"), Pin->PinType.PinCategory.IsNone() ? TEXT("None") : Pin->PinType.PinCategory.ToString());
            PinInfo->SetStringField(TEXT("pin_sub_category"), Pin->PinType.PinSubCategory.IsNone() ? TEXT("None") : Pin->PinType.PinSubCategory.ToString());
            PinInfo->SetBoolField(TEXT("is_reference"), Pin->PinType.bIsReference);
            PinInfo->SetBoolField(TEXT("is_array"), Pin->PinType.ContainerType == EPinContainerType::Array);
            PinInfo->SetBoolField(TEXT("is_map"), Pin->PinType.ContainerType == EPinContainerType::Map);
            PinInfo->SetBoolField(TEXT("is_set"), Pin->PinType.ContainerType == EPinContainerType::Set);

            FString DefaultValue = Pin->GetDefaultAsString();
            if (!DefaultValue.IsEmpty())
            {
                PinInfo->SetStringField(TEXT("default_value"), DefaultValue);
            }

            PinInfo->SetNumberField(TEXT("linked_count"), Pin->LinkedTo.Num());

            if (Pin->LinkedTo.Num() > 0)
            {
                TArray<TSharedPtr<FJsonValue>> LinkedPinsArray;
                for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
                {
                    if (LinkedPin && LinkedPin->GetOwningNode())
                    {
                        TSharedPtr<FJsonObject> LinkedInfo = MakeShareable(new FJsonObject);
                        LinkedInfo->SetStringField(TEXT("node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
                        LinkedInfo->SetStringField(TEXT("node_name"), LinkedPin->GetOwningNode()->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
                        LinkedInfo->SetStringField(TEXT("pin_id"), LinkedPin->PinId.ToString());
                        LinkedInfo->SetStringField(TEXT("pin_name"), LinkedPin->PinName);
                        LinkedInfo->SetStringField(TEXT("direction"), LinkedPin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));
                        LinkedPinsArray.Add(MakeShareable(new FJsonValueObject(LinkedInfo)));
                    }
                }
                PinInfo->SetArrayField(TEXT("linked_to"), LinkedPinsArray);
            }

            PinsArray.Add(MakeShareable(new FJsonValueObject(PinInfo)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("node_name"), TargetNode->GetName());
        Result->SetStringField(TEXT("node_display_name"), TargetNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
        Result->SetStringField(TEXT("node_type"), TargetNode->GetClass()->GetName());
        Result->SetStringField(TEXT("graph_name"), FoundGraph ? FoundGraph->GetName() : TEXT("Unknown"));
        Result->SetArrayField(TEXT("pins"), PinsArray);
        Result->SetNumberField(TEXT("pin_count"), PinsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAnimNodePinsHandler)