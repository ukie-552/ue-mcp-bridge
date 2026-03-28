#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpConnectBlueprintPinsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("connect_blueprint_pins"); }

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
        if (!Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'target_node_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString TargetPinName;
        if (!Params->TryGetStringField(TEXT("target_pin_name"), TargetPinName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'target_pin_name' parameter"), TEXT("MISSING_PARAM"));
        }

        UBlueprint* Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UEdGraphPin* SourcePin = FMcpBlueprintUtils::FindPin(Blueprint, SourceNodeId, SourcePinName);
        if (!SourcePin)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Source pin not found: %s"), *SourcePinName),
                TEXT("PIN_NOT_FOUND")
            );
        }

        UEdGraphPin* TargetPin = FMcpBlueprintUtils::FindPin(Blueprint, TargetNodeId, TargetPinName);
        if (!TargetPin)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Target pin not found: %s"), *TargetPinName),
                TEXT("PIN_NOT_FOUND")
            );
        }

        const UEdGraphSchema* Schema = SourcePin->GetOwningNode()->GetGraph()->GetSchema();
        if (!Schema)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to get graph schema"), TEXT("SCHEMA_ERROR"));
        }

        FPinConnectionResponse Response = Schema->CanCreateConnection(SourcePin, TargetPin);
        if (Response.Response != CONNECT_RESPONSE_MAKE)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Cannot connect pins: %s"), *Response.Message.ToString()),
                TEXT("CONNECTION_INVALID")
            );
        }

        FName ExecCategory = FName(TEXT("exec"));
        if (TargetPin->LinkedTo.Num() > 0 && TargetPin->PinType.PinCategory != ExecCategory)
        {
            TargetPin->BreakAllPinLinks();
        }

        if (!Schema->TryCreateConnection(SourcePin, TargetPin))
        {
            return FMcpCommandResult::Failure(TEXT("Failed to create connection"), TEXT("CONNECTION_FAILED"));
        }

        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("connected"), true);
        Result->SetStringField(TEXT("source_node_id"), SourceNodeId);
        Result->SetStringField(TEXT("source_pin_name"), SourcePinName);
        Result->SetStringField(TEXT("target_node_id"), TargetNodeId);
        Result->SetStringField(TEXT("target_pin_name"), TargetPinName);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpConnectBlueprintPinsHandler)
