#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpConnectAnimNodePinsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("connect_anim_node_pins"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString SourceNodeId;
        if (!Params->TryGetStringField(TEXT("source_node_id"), SourceNodeId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'source_node_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString SourcePinName;
        if (!Params->TryGetStringField(TEXT("source_pin_name"), SourcePinName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'source_pin_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString TargetNodeId;
        if (!Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'target_node_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString TargetPinName;
        if (!Params->TryGetStringField(TEXT("target_pin_name"), TargetPinName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'target_pin_name' parameter"), TEXT("MISSING_PARAM"));
        }

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        FGuid SourceGuid, TargetGuid;
        if (!FGuid::Parse(SourceNodeId, SourceGuid) || !FGuid::Parse(TargetNodeId, TargetGuid))
        {
            return FMcpCommandResult::Failure(TEXT("Invalid node_id format"), TEXT("INVALID_GUID"));
        }

        UEdGraphPin* SourcePin = nullptr;
        UEdGraphPin* TargetPin = nullptr;
        UEdGraph* FoundGraph = nullptr;

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

                if (Node->NodeGuid == SourceGuid)
                {
                    for (UEdGraphPin* Pin : Node->Pins)
                    {
                        if (Pin && Pin->PinName == SourcePinName)
                        {
                            SourcePin = Pin;
                            FoundGraph = Graph;
                            break;
                        }
                    }
                }

                if (Node->NodeGuid == TargetGuid)
                {
                    for (UEdGraphPin* Pin : Node->Pins)
                    {
                        if (Pin && Pin->PinName == TargetPinName)
                        {
                            TargetPin = Pin;
                            if (FoundGraph) break;
                        }
                    }
                }

                if (SourcePin && TargetPin) break;
            }

            if (SourcePin && TargetPin) break;
        }

        if (!SourcePin || !TargetPin)
        {
            return FMcpCommandResult::Failure(TEXT("Pins not found"), TEXT("PIN_NOT_FOUND"));
        }

        if (SourcePin->Direction == TargetPin->Direction)
        {
            return FMcpCommandResult::Failure(TEXT("Cannot connect pins with same direction"), TEXT("INVALID_CONNECTION"));
        }

        if (!FoundGraph)
        {
            return FMcpCommandResult::Failure(TEXT("Graph not found"), TEXT("GRAPH_NOT_FOUND"));
        }

        FoundGraph->Modify();

        if (SourcePin->Direction == EGPD_Output)
        {
            SourcePin->LinkedTo.Add(TargetPin);
        }
        else
        {
            TargetPin->LinkedTo.Add(SourcePin);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("connected"), true);
        Result->SetStringField(TEXT("source_node_id"), SourceNodeId);
        Result->SetStringField(TEXT("source_pin_name"), SourcePinName);
        Result->SetStringField(TEXT("target_node_id"), TargetNodeId);
        Result->SetStringField(TEXT("target_pin_name"), TargetPinName);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpConnectAnimNodePinsHandler)