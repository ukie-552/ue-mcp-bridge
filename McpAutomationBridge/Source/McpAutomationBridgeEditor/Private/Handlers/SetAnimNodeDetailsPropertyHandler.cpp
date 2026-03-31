#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_Base.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpSetAnimNodeDetailsPropertyHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_anim_node_details_property"); }

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

        UAnimGraphNode_Base* AnimNode = Cast<UAnimGraphNode_Base>(TargetNode);
        if (!AnimNode)
        {
            return FMcpCommandResult::Failure(
                TEXT("Node is not an animation node type"),
                TEXT("INVALID_NODE_TYPE")
            );
        }

        UObject* AnimObject = AnimNode->GetAnimNode();
        if (!AnimObject)
        {
            return FMcpCommandResult::Failure(
                TEXT("Failed to get animation node object"),
                TEXT("ANIM_NODE_NULL")
            );
        }

        FProperty* TargetProperty = AnimObject->GetClass()->FindPropertyByName(*PropertyName);
        if (!TargetProperty)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Property not found: %s"), *PropertyName),
                TEXT("PROPERTY_NOT_FOUND")
            );
        }

        if (TargetProperty->HasAnyPropertyFlags(CPF_EditConst))
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Property is read-only: %s"), *PropertyName),
                TEXT("PROPERTY_READ_ONLY")
            );
        }

        bool bSuccess = SetPropertyValue(TargetProperty, AnimObject, PropertyValue);

        if (!bSuccess)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to set property value: %s"), *PropertyName),
                TEXT("SET_PROPERTY_FAILED")
            );
        }

        FBlueprintEditorUtils::MarkBlueprintAsModified(AnimBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("property_name"), PropertyName);
        Result->SetStringField(TEXT("property_value"), PropertyValue);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);

        return FMcpCommandResult::Success(Result);
    }

private:
    bool SetPropertyValue(FProperty* Property, UObject* Object, const FString& Value)
    {
        if (!Property || !Object) return false;

        void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);

        if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
        {
            BoolProp->SetPropertyValue(ValuePtr, Value.ToBool());
            return true;
        }
        else if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
        {
            IntProp->SetPropertyValue(ValuePtr, FCString::Atoi(*Value));
            return true;
        }
        else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
        {
            FloatProp->SetPropertyValue(ValuePtr, FCString::Atof(*Value));
            return true;
        }
        else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Property))
        {
            DoubleProp->SetPropertyValue(ValuePtr, FCString::Atod(*Value));
            return true;
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
        {
            StrProp->SetPropertyValue(ValuePtr, Value);
            return true;
        }
        else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
        {
            NameProp->SetPropertyValue(ValuePtr, FName(*Value));
            return true;
        }
        else if (FTextProperty* TextProp = CastField<FTextProperty>(Property))
        {
            TextProp->SetPropertyValue(ValuePtr, FText::FromString(Value));
            return true;
        }
        else if (FByteProperty* ByteProp = CastField<FByteProperty>(Property))
        {
            if (ByteProp->Enum)
            {
                int64 EnumValue = ByteProp->Enum->GetValueByNameString(Value);
                if (EnumValue != INDEX_NONE)
                {
                    ByteProp->SetPropertyValue(ValuePtr, (uint8)EnumValue);
                    return true;
                }
            }
            ByteProp->SetPropertyValue(ValuePtr, (uint8)FCString::Atoi(*Value));
            return true;
        }
        else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
        {
            int64 EnumValue = EnumProp->GetEnum()->GetValueByNameString(Value);
            if (EnumValue != INDEX_NONE)
            {
                EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, EnumValue);
                return true;
            }
            return false;
        }
        else if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Property))
        {
            if (Value == TEXT("None") || Value.IsEmpty())
            {
                ObjProp->SetObjectPropertyValue(ValuePtr, nullptr);
                return true;
            }
            UObject* Obj = LoadObject<UObject>(nullptr, *Value);
            ObjProp->SetObjectPropertyValue(ValuePtr, Obj);
            return true;
        }
        else if (FClassProperty* ClassProp = CastField<FClassProperty>(Property))
        {
            if (Value == TEXT("None") || Value.IsEmpty())
            {
                ClassProp->SetPropertyValue(ValuePtr, nullptr);
                return true;
            }
            UClass* Class = LoadClass<UObject>(nullptr, *Value);
            ClassProp->SetPropertyValue(ValuePtr, Class);
            return true;
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
        {
            if (StructProp->Struct == TBaseStructure<FVector>::Get())
            {
                FVector Vec = ParseVector(Value);
                StructProp->SetStructPropertyValue(ValuePtr, &Vec);
                return true;
            }
            else if (StructProp->Struct == TBaseStructure<FRotator>::Get())
            {
                FRotator Rot = ParseRotator(Value);
                StructProp->SetStructPropertyValue(ValuePtr, &Rot);
                return true;
            }
            else if (StructProp->Struct == TBaseStructure<FTransform>::Get())
            {
                return false;
            }
            else if (StructProp->Struct == TBaseStructure<FLinearColor>::Get())
            {
                FLinearColor LC = ParseLinearColor(Value);
                StructProp->SetStructPropertyValue(ValuePtr, &LC);
                return true;
            }
            else if (StructProp->Struct == TBaseStructure<FColor>::Get())
            {
                FColor C = ParseColor(Value);
                StructProp->SetStructPropertyValue(ValuePtr, &C);
                return true;
            }
        }
        else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
        {
            return false;
        }

        return false;
    }

    FVector ParseVector(const FString& Value)
    {
        FVector Vec = FVector::ZeroVector;

        if (Value.Contains(TEXT("X=")) && Value.Contains(TEXT("Y=")) && Value.Contains(TEXT("Z=")))
        {
            FString XStr, YStr, ZStr;

            if (Value.Split(TEXT("X="), nullptr, &XStr))
            {
                XStr.Split(TEXT(","), nullptr, &YStr);
                Vec.X = FCString::Atof(*XStr);
            }
            if (YStr.Split(TEXT("Y="), nullptr, &YStr))
            {
                YStr.Split(TEXT(","), nullptr, &ZStr);
                Vec.Y = FCString::Atof(*YStr);
            }
            if (!ZStr.IsEmpty())
            {
                ZStr.Split(TEXT(","), nullptr, nullptr);
                Vec.Z = FCString::Atof(*ZStr);
            }
        }

        return Vec;
    }

    FRotator ParseRotator(const FString& Value)
    {
        FRotator Rot = FRotator::ZeroRotator;

        if (Value.Contains(TEXT("P=")) && Value.Contains(TEXT("Y=")) && Value.Contains(TEXT("R=")))
        {
            FString PStr, YStr, RStr;

            if (Value.Split(TEXT("P="), nullptr, &PStr))
            {
                PStr.Split(TEXT(","), nullptr, &YStr);
                Rot.Pitch = FCString::Atof(*PStr);
            }
            if (YStr.Split(TEXT("Y="), nullptr, &YStr))
            {
                YStr.Split(TEXT(","), nullptr, &RStr);
                Rot.Yaw = FCString::Atof(*YStr);
            }
            if (!RStr.IsEmpty())
            {
                Rot.Roll = FCString::Atof(*RStr);
            }
        }

        return Rot;
    }

    FLinearColor ParseLinearColor(const FString& Value)
    {
        FLinearColor LC = FLinearColor::White;

        if (Value.Contains(TEXT("R=")) && Value.Contains(TEXT("G=")) && Value.Contains(TEXT("B=")))
        {
            FString RStr, GStr, BStr, AStr;

            if (Value.Split(TEXT("R="), nullptr, &RStr))
            {
                RStr.Split(TEXT(","), nullptr, &GStr);
                LC.R = FCString::Atof(*RStr);
            }
            if (GStr.Split(TEXT("G="), nullptr, &GStr))
            {
                GStr.Split(TEXT(","), nullptr, &BStr);
                LC.G = FCString::Atof(*GStr);
            }
            if (BStr.Split(TEXT("B="), nullptr, &BStr))
            {
                BStr.Split(TEXT(","), nullptr, &AStr);
                LC.B = FCString::Atof(*BStr);
            }
            if (!AStr.IsEmpty())
            {
                LC.A = FCString::Atof(*AStr);
            }
        }

        return LC;
    }

    FColor ParseColor(const FString& Value)
    {
        FColor C = FColor::White;

        if (Value.Contains(TEXT("R=")) && Value.Contains(TEXT("G=")) && Value.Contains(TEXT("B=")))
        {
            FString RStr, GStr, BStr, AStr;

            if (Value.Split(TEXT("R="), nullptr, &RStr))
            {
                RStr.Split(TEXT(","), nullptr, &GStr);
                C.R = FCString::Atoi(*RStr);
            }
            if (GStr.Split(TEXT("G="), nullptr, &GStr))
            {
                GStr.Split(TEXT(","), nullptr, &BStr);
                C.G = FCString::Atoi(*GStr);
            }
            if (BStr.Split(TEXT("B="), nullptr, &BStr))
            {
                BStr.Split(TEXT(","), nullptr, &AStr);
                C.B = FCString::Atoi(*BStr);
            }
            if (!AStr.IsEmpty())
            {
                C.A = FCString::Atoi(*AStr);
            }
        }

        return C;
    }
};

REGISTER_MCP_COMMAND(FMcpSetAnimNodeDetailsPropertyHandler)