#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_Base.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AnimNode_BlendSpacePlayer.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "Animation/AnimNode_Root.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

class FMcpSetAnimNodePinValueHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_anim_node_pin_value"); }

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
        if (!Params->TryGetStringField(TEXT("pin_name"), PinName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'pin_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString PinValue;
        if (!Params->TryGetStringField(TEXT("pin_value"), PinValue))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'pin_value' parameter"), TEXT("MISSING_PARAM"));
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
        UEdGraphPin* TargetPin = nullptr;

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
                    for (UEdGraphPin* Pin : Node->Pins)
                    {
                        if (Pin && Pin->PinName == PinName)
                        {
                            TargetPin = Pin;
                            break;
                        }
                    }
                    break;
                }
            }

            if (TargetNode && TargetPin) break;
        }

        if (!TargetNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Node not found: %s"), *NodeId),
                TEXT("NODE_NOT_FOUND")
            );
        }

        if (!TargetPin)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Pin not found: %s"), *PinName),
                TEXT("PIN_NOT_FOUND")
            );
        }

        bool bSuccess = SetPinDefaultValue(TargetPin, PinValue);

        if (!bSuccess)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to set pin value for: %s"), *PinName),
                TEXT("SET_PIN_VALUE_FAILED")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), true);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("pin_name"), PinName);
        Result->SetStringField(TEXT("pin_value"), PinValue);

        return FMcpCommandResult::Success(Result);
    }

private:
    bool SetPinDefaultValue(UEdGraphPin* Pin, const FString& Value)
    {
        if (!Pin) return false;

        const FEdGraphPinType& PinType = Pin->PinType;

        if (PinType.PinCategory == TEXT("bool"))
        {
            Pin->DefaultValue = Value.ToBool() ? TEXT("true") : TEXT("false");
            return true;
        }
        else if (PinType.PinCategory == TEXT("int") || PinType.PinCategory == TEXT("byte"))
        {
            Pin->DefaultValue = Value;
            return true;
        }
        else if (PinType.PinCategory == TEXT("float"))
        {
            Pin->DefaultValue = Value;
            return true;
        }
        else if (PinType.PinCategory == TEXT("string") || PinType.PinCategory == TEXT("name"))
        {
            Pin->DefaultValue = Value;
            return true;
        }
        else if (PinType.PinCategory == TEXT("text"))
        {
            Pin->DefaultValue = Value;
            return true;
        }
        else if (PinType.PinCategory == TEXT("object") || PinType.PinCategory == TEXT("class"))
        {
            Pin->DefaultObjectName = FName(*Value);
            Pin->DefaultValue = Value;
            return true;
        }
        else if (PinType.PinCategory == TEXT("struct"))
        {
            Pin->DefaultValue = Value;
            return true;
        }
        else if (PinType.PinCategory == TEXT("vector"))
        {
            Pin->DefaultValue = Value;
            return true;
        }
        else if (PinType.PinCategory == TEXT("rotator"))
        {
            Pin->DefaultValue = Value;
            return true;
        }
        else if (PinType.PinCategory == TEXT("transform"))
        {
            Pin->DefaultValue = Value;
            return true;
        }
        else if (PinType.PinCategory == TEXT("linearcolor"))
        {
            Pin->DefaultValue = Value;
            return true;
        }

        Pin->DefaultValue = Value;
        return true;
    }
};

REGISTER_MCP_COMMAND(FMcpSetAnimNodePinValueHandler)