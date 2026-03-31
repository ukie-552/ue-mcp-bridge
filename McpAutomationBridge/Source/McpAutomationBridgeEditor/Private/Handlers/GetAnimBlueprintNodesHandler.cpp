#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_Base.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

class FMcpGetAnimBlueprintNodesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_anim_blueprint_nodes"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString GraphName;
        Params->TryGetStringField(TEXT("graph_name"), GraphName);

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<UEdGraph*> TargetGraphs;
        if (!GraphName.IsEmpty())
        {
            for (UEdGraph* Graph : AnimBP->UbergraphPages)
            {
                if (Graph && Graph->GetName() == GraphName)
                {
                    TargetGraphs.Add(Graph);
                    break;
                }
            }
        }
        else
        {
            TargetGraphs.Append(AnimBP->UbergraphPages);
            TargetGraphs.Append(AnimBP->FunctionGraphs);
            TargetGraphs.Append(AnimBP->MacroGraphs);
        }

        TArray<TSharedPtr<FJsonValue>> NodesArray;

        for (UEdGraph* Graph : TargetGraphs)
        {
            if (!Graph) continue;

            FString GraphNameStr = Graph->GetName();

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (!Node) continue;

                TSharedPtr<FJsonObject> NodeInfo = MakeShareable(new FJsonObject);
                NodeInfo->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
                NodeInfo->SetStringField(TEXT("node_name"), Node->GetName());
                NodeInfo->SetStringField(TEXT("node_type"), Node->GetClass()->GetName());
                NodeInfo->SetStringField(TEXT("graph_name"), GraphNameStr);
                NodeInfo->SetNumberField(TEXT("pos_x"), Node->NodePosX);
                NodeInfo->SetNumberField(TEXT("pos_y"), Node->NodePosY);
                NodeInfo->SetStringField(TEXT("display_name"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());

                TArray<TSharedPtr<FJsonValue>> PinsArray;
                for (UEdGraphPin* Pin : Node->Pins)
                {
                    if (!Pin) continue;

                    TSharedPtr<FJsonObject> PinInfo = MakeShareable(new FJsonObject);
                    PinInfo->SetStringField(TEXT("pin_id"), Pin->PinId.ToString());
                    PinInfo->SetStringField(TEXT("pin_name"), Pin->PinName);
                    PinInfo->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));
                    PinInfo->SetStringField(TEXT("pin_type"), Pin->PinType.PinCategory);
                    PinInfo->SetStringField(TEXT("pin_sub_category"), Pin->PinType.PinSubCategory);

                    if (Pin->LinkedTo.Num() > 0)
                    {
                        TArray<TSharedPtr<FJsonValue>> LinkedPinsArray;
                        for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
                        {
                            if (LinkedPin)
                            {
                                TSharedPtr<FJsonObject> LinkedPinInfo = MakeShareable(new FJsonObject);
                                LinkedPinInfo->SetStringField(TEXT("node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
                                LinkedPinInfo->SetStringField(TEXT("pin_name"), LinkedPin->PinName);
                                LinkedPinsArray.Add(MakeShareable(new FJsonValueObject(LinkedPinInfo)));
                            }
                        }
                        PinInfo->SetArrayField(TEXT("linked_to"), LinkedPinsArray);
                    }

                    PinsArray.Add(MakeShareable(new FJsonValueObject(PinInfo)));
                }
                NodeInfo->SetArrayField(TEXT("pins"), PinsArray);
                NodeInfo->SetNumberField(TEXT("pin_count"), PinsArray.Num());

                NodesArray.Add(MakeShareable(new FJsonValueObject(NodeInfo)));
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetArrayField(TEXT("nodes"), NodesArray);
        Result->SetNumberField(TEXT("node_count"), NodesArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAnimBlueprintNodesHandler)