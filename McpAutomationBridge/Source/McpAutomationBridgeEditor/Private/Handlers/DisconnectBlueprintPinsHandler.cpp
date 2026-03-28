#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpDisconnectBlueprintPinsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("disconnect_blueprint_pins"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
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
        FString TargetPinName;
        Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId);
        Params->TryGetStringField(TEXT("target_pin_name"), TargetPinName);

        UBlueprint* Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UEdGraphNode* SourceNode = FMcpBlueprintUtils::FindNodeByGuid(Blueprint, SourceNodeId);
        if (!SourceNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Source node not found: %s"), *SourceNodeId),
                TEXT("NODE_NOT_FOUND")
            );
        }

        UEdGraphPin* SourcePin = SourceNode->FindPin(*SourcePinName);
        if (!SourcePin)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Source pin not found: %s"), *SourcePinName),
                TEXT("PIN_NOT_FOUND")
            );
        }

        int32 DisconnectedCount = 0;

        if (!TargetNodeId.IsEmpty() && !TargetPinName.IsEmpty())
        {
            UEdGraphNode* TargetNode = FMcpBlueprintUtils::FindNodeByGuid(Blueprint, TargetNodeId);
            if (!TargetNode)
            {
                return FMcpCommandResult::Failure(
                    FString::Printf(TEXT("Target node not found: %s"), *TargetNodeId),
                    TEXT("NODE_NOT_FOUND")
                );
            }

            UEdGraphPin* TargetPin = TargetNode->FindPin(*TargetPinName);
            if (!TargetPin)
            {
                return FMcpCommandResult::Failure(
                    FString::Printf(TEXT("Target pin not found: %s"), *TargetPinName),
                    TEXT("PIN_NOT_FOUND")
                );
            }

            SourcePin->BreakLinkTo(TargetPin);
            DisconnectedCount = 1;
        }
        else
        {
            DisconnectedCount = SourcePin->LinkedTo.Num();
            SourcePin->BreakAllPinLinks();
        }

        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("disconnected"), true);
        Result->SetNumberField(TEXT("disconnected_count"), DisconnectedCount);
        Result->SetStringField(TEXT("source_node_id"), SourceNodeId);
        Result->SetStringField(TEXT("source_pin_name"), SourcePinName);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpDisconnectBlueprintPinsHandler)
