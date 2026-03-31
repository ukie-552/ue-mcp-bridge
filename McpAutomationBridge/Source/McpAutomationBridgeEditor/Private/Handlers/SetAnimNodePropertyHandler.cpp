#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_Base.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AnimNode_BlendSpacePlayer.h"
#include "AnimGraphNode_SequencePlayer.h"
#include "AnimGraphNode_BlendSpacePlayer.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "UObject/UnrealType.h"

class FMcpSetAnimNodePropertyHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_anim_node_property"); }

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

        FString PropertyName;
        if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'property_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString PropertyValue;
        if (!Params->TryGetStringField(TEXT("property_value"), PropertyValue))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'property_value' parameter"), TEXT("MISSING_PARAM"));
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

        UAnimGraphNode_Base* TargetNode = nullptr;
        TArray<UEdGraph*> AllGraphs;
        AllGraphs.Append(AnimBP->UbergraphPages);
        AllGraphs.Append(AnimBP->FunctionGraphs);
        AllGraphs.Append(AnimBP->MacroGraphs);

        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (Node && Node->NodeGuid == NodeGuid)
                {
                    TargetNode = Cast<UAnimGraphNode_Base>(Node);
                    break;
                }
            }

            if (TargetNode) break;
        }

        if (!TargetNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation node not found: %s"), *NodeId),
                TEXT("NODE_NOT_FOUND")
            );
        }

        bool bPropertySet = false;

        if (UAnimGraphNode_SequencePlayer* SeqPlayer = Cast<UAnimGraphNode_SequencePlayer>(TargetNode))
        {
            bPropertySet = SetSequencePlayerProperty(SeqPlayer, PropertyName, PropertyValue);
        }
        else if (UAnimGraphNode_BlendSpacePlayer* BlendSpacePlayer = Cast<UAnimGraphNode_BlendSpacePlayer>(TargetNode))
        {
            bPropertySet = SetBlendSpacePlayerProperty(BlendSpacePlayer, PropertyName, PropertyValue);
        }
        else
        {
            bPropertySet = SetGenericProperty(TargetNode, PropertyName, PropertyValue);
        }

        if (!bPropertySet)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to set property: %s"), *PropertyName),
                TEXT("PROPERTY_SET_FAILED")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), true);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("property_name"), PropertyName);
        Result->SetStringField(TEXT("property_value"), PropertyValue);

        return FMcpCommandResult::Success(Result);
    }

private:
    bool SetSequencePlayerProperty(UAnimGraphNode_SequencePlayer* Node, const FString& PropertyName, const FString& Value)
    {
        FAnimNode_SequencePlayer& AnimNode = Node->Node;

        if (PropertyName == TEXT("bLoop"))
        {
            AnimNode.bLoop = Value.ToBool();
            return true;
        }
        else if (PropertyName == TEXT("PlayRate"))
        {
            AnimNode.PlayRate = FCString::Atof(*Value);
            return true;
        }
        else if (PropertyName == TEXT("bPauseWith_HighestPriority"))
        {
            AnimNode.bPauseWith_HighestPriority = Value.ToBool();
            return true;
        }
        else if (PropertyName == TEXT("StartTime"))
        {
            AnimNode.StartTime = FCString::Atof(*Value);
            return true;
        }

        return false;
    }

    bool SetBlendSpacePlayerProperty(UAnimGraphNode_BlendSpacePlayer* Node, const FString& PropertyName, const FString& Value)
    {
        FAnimNode_BlendSpacePlayer& AnimNode = Node->Node;

        if (PropertyName == TEXT("bLoop"))
        {
            AnimNode.bLoop = Value.ToBool();
            return true;
        }
        else if (PropertyName == TEXT("PlayRate"))
        {
            AnimNode.PlayRate = FCString::Atof(*Value);
            return true;
        }
        else if (PropertyName == TEXT("X"))
        {
            AnimNode.NormalizedBlendSpace.X = FCString::Atof(*Value);
            return true;
        }
        else if (PropertyName == TEXT("Y"))
        {
            AnimNode.NormalizedBlendSpace.Y = FCString::Atof(*Value);
            return true;
        }

        return false;
    }

    bool SetGenericProperty(UAnimGraphNode_Base* Node, const FString& PropertyName, const FString& Value)
    {
        UStruct* Struct = Node->GetClass();
        FProperty* Property = FindFProperty<FProperty>(Struct, *PropertyName);

        if (!Property)
        {
            return false;
        }

        void* PropertyPtr = Property->ContainerPtrToValuePtr<void>(Node);
        if (!PropertyPtr)
        {
            return false;
        }

        if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
        {
            BoolProp->SetPropertyValue(PropertyPtr, Value.ToBool());
            return true;
        }
        else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
        {
            FloatProp->SetPropertyValue(PropertyPtr, FCString::Atof(*Value));
            return true;
        }
        else if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
        {
            IntProp->SetPropertyValue(PropertyPtr, FCString::Atoi(*Value));
            return true;
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
        {
            StrProp->SetPropertyValue(PropertyPtr, Value);
            return true;
        }
        else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
        {
            NameProp->SetPropertyValue(PropertyPtr, *Value);
            return true;
        }

        return false;
    }
};

REGISTER_MCP_COMMAND(FMcpSetAnimNodePropertyHandler)