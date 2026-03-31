#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_Base.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"

class FMcpGetAnimNodeDetailsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_anim_node_details"); }

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

        FString CategoryFilter;
        Params->TryGetStringField(TEXT("category"), CategoryFilter);

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

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);

        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetStringField(TEXT("node_id"), NodeId);
        Result->SetStringField(TEXT("node_name"), TargetNode->GetName());
        Result->SetStringField(TEXT("node_display_name"), TargetNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
        Result->SetStringField(TEXT("node_type"), TargetNode->GetClass()->GetName());
        Result->SetStringField(TEXT("graph_name"), FoundGraph ? FoundGraph->GetName() : TEXT("Unknown"));

        UAnimGraphNode_Base* AnimNode = Cast<UAnimGraphNode_Base>(TargetNode);
        if (AnimNode)
        {
            UObject* AnimObject = AnimNode->GetAnimNode();
            if (AnimObject)
            {
                Result->SetStringField(TEXT("anim_node_class"), AnimObject->GetClass()->GetName());

                TArray<TSharedPtr<FJsonValue>> PropertiesArray;
                TArray<TSharedPtr<FJsonValue>> CategoriesArray;
                TSet<FString> FoundCategories;

                UClass* NodeClass = AnimObject->GetClass();
                for (TFieldIterator<FProperty> PropIt(NodeClass); PropIt; ++PropIt)
                {
                    FProperty* Property = *PropIt;

                    if (!Property->HasAnyPropertyFlags(CPF_Edit))
                    {
                        continue;
                    }

                    FString PropertyCategory = Property->GetMetaData(TEXT("Category"));
                    if (PropertyCategory.IsEmpty())
                    {
                        PropertyCategory = TEXT("Default");
                    }

                    if (!CategoryFilter.IsEmpty() && PropertyCategory != CategoryFilter)
                    {
                        continue;
                    }

                    if (!FoundCategories.Contains(PropertyCategory))
                    {
                        FoundCategories.Add(PropertyCategory);
                        CategoriesArray.Add(MakeShareable(new FJsonValueString(PropertyCategory)));
                    }

                    TSharedPtr<FJsonObject> PropInfo = MakeShareable(new FJsonObject);
                    PropInfo->SetStringField(TEXT("name"), Property->GetName());
                    PropInfo->SetStringField(TEXT("display_name"), Property->GetDisplayNameText().ToString());
                    PropInfo->SetStringField(TEXT("category"), PropertyCategory);
                    PropInfo->SetStringField(TEXT("type"), GetPropertyType(Property));
                    PropInfo->SetStringField(TEXT("tooltip"), Property->GetToolTipText().ToString());
                    PropInfo->SetBoolField(TEXT("is_read_only"), Property->HasAnyPropertyFlags(CPF_EditConst));
                    PropInfo->SetBoolField(TEXT("is_array"), Property->IsA<FArrayProperty>());
                    PropInfo->SetBoolField(TEXT("is_struct"), Property->IsA<FStructProperty>());
                    PropInfo->SetBoolField(TEXT("is_enum"), Property->IsA<FEnumProperty>());
                    PropInfo->SetBoolField(TEXT("is_object"), Property->IsA<FObjectProperty>());

                    FString CurrentValue = GetPropertyValueAsString(Property, AnimObject);
                    PropInfo->SetStringField(TEXT("current_value"), CurrentValue);

                    FString DefaultValue = Property->GetMetaData(TEXT("DefaultValue"));
                    if (!DefaultValue.IsEmpty())
                    {
                        PropInfo->SetStringField(TEXT("default_value"), DefaultValue);
                    }

                    TSharedPtr<FJsonObject> MetaData = MakeShareable(new FJsonObject);
                    FString UIMin = Property->GetMetaData(TEXT("UIMin"));
                    FString UIMax = Property->GetMetaData(TEXT("UIMax"));
                    FString ClampMin = Property->GetMetaData(TEXT("ClampMin"));
                    FString ClampMax = Property->GetMetaData(TEXT("ClampMax"));

                    if (!UIMin.IsEmpty()) MetaData->SetStringField(TEXT("ui_min"), UIMin);
                    if (!UIMax.IsEmpty()) MetaData->SetStringField(TEXT("ui_max"), UIMax);
                    if (!ClampMin.IsEmpty()) MetaData->SetStringField(TEXT("clamp_min"), ClampMin);
                    if (!ClampMax.IsEmpty()) MetaData->SetStringField(TEXT("clamp_max"), ClampMax);

                    if (MetaData->Values.Num() > 0)
                    {
                        PropInfo->SetObjectField(TEXT("metadata"), MetaData);
                    }

                    PropertiesArray.Add(MakeShareable(new FJsonValueObject(PropInfo)));
                }

                Result->SetArrayField(TEXT("properties"), PropertiesArray);
                Result->SetArrayField(TEXT("categories"), CategoriesArray);
                Result->SetNumberField(TEXT("property_count"), PropertiesArray.Num());
            }
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
                        LinkedInfo->SetStringField(TEXT("pin_name"), LinkedPin->PinName);
                        LinkedPinsArray.Add(MakeShareable(new FJsonValueObject(LinkedInfo)));
                    }
                }
                PinInfo->SetArrayField(TEXT("linked_to"), LinkedPinsArray);
            }

            PinsArray.Add(MakeShareable(new FJsonValueObject(PinInfo)));
        }

        Result->SetArrayField(TEXT("pins"), PinsArray);
        Result->SetNumberField(TEXT("pin_count"), PinsArray.Num());

        return FMcpCommandResult::Success(Result);
    }

private:
    FString GetPropertyType(FProperty* Property)
    {
        if (!Property) return TEXT("Unknown");

        if (Property->IsA<FBoolProperty>()) return TEXT("bool");
        if (Property->IsA<FIntProperty>()) return TEXT("int");
        if (Property->IsA<FFloatProperty>()) return TEXT("float");
        if (Property->IsA<FDoubleProperty>()) return TEXT("double");
        if (Property->IsA<FStrProperty>()) return TEXT("string");
        if (Property->IsA<FNameProperty>()) return TEXT("name");
        if (Property->IsA<FTextProperty>()) return TEXT("text");
        if (Property->IsA<FEnumProperty>()) return TEXT("enum");
        if (Property->IsA<FByteProperty>()) return TEXT("byte");
        if (Property->IsA<FObjectProperty>()) return TEXT("object");
        if (Property->IsA<FClassProperty>()) return TEXT("class");
        if (Property->IsA<FStructProperty>()) return TEXT("struct");
        if (Property->IsA<FArrayProperty>()) return TEXT("array");
        if (Property->IsA<FMapProperty>()) return TEXT("map");
        if (Property->IsA<FSetProperty>()) return TEXT("set");

        return Property->GetClass()->GetName();
    }

    FString GetPropertyValueAsString(FProperty* Property, UObject* Object)
    {
        if (!Property || !Object) return TEXT("");

        void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);

        if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Property))
        {
            if (NumericProp->IsFloatingPoint())
            {
                return FString::SanitizeFloat(NumericProp->GetFloatingPointPropertyValue(ValuePtr));
            }
            else
            {
                return FString::Printf(TEXT("%lld"), NumericProp->GetSignedIntPropertyValue(ValuePtr));
            }
        }
        else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
        {
            return BoolProp->GetPropertyValue(ValuePtr) ? TEXT("true") : TEXT("false");
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
        {
            return StrProp->GetPropertyValue(ValuePtr);
        }
        else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
        {
            return NameProp->GetPropertyValue(ValuePtr).ToString();
        }
        else if (FTextProperty* TextProp = CastField<FTextProperty>(Property))
        {
            return TextProp->GetPropertyValue(ValuePtr).ToString();
        }
        else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
        {
            return EnumProp->GetEnum()->GetNameStringByValue(EnumProp->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr));
        }
        else if (FByteProperty* ByteProp = CastField<FByteProperty>(Property))
        {
            if (ByteProp->Enum)
            {
                return ByteProp->Enum->GetNameStringByValue(ByteProp->GetPropertyValue(ValuePtr));
            }
            return FString::Printf(TEXT("%d"), ByteProp->GetPropertyValue(ValuePtr));
        }
        else if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Property))
        {
            UObject* Obj = ObjProp->GetObjectPropertyValue(ValuePtr);
            return Obj ? Obj->GetName() : TEXT("None");
        }
        else if (FClassProperty* ClassProp = CastField<FClassProperty>(Property))
        {
            UClass* Class = ClassProp->GetPropertyValue(ValuePtr);
            return Class ? Class->GetName() : TEXT("None");
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
        {
            if (StructProp->Struct == TBaseStructure<FVector>::Get())
            {
                FVector* Vec = static_cast<FVector*>(ValuePtr);
                return FString::Printf(TEXT("(X=%f, Y=%f, Z=%f)"), Vec->X, Vec->Y, Vec->Z);
            }
            else if (StructProp->Struct == TBaseStructure<FRotator>::Get())
            {
                FRotator* Rot = static_cast<FRotator*>(ValuePtr);
                return FString::Printf(TEXT("(P=%f, Y=%f, R=%f)"), Rot->Pitch, Rot->Yaw, Rot->Roll);
            }
            else if (StructProp->Struct == TBaseStructure<FTransform>::Get())
            {
                return TEXT("Transform");
            }
            else if (StructProp->Struct == TBaseStructure<FLinearColor>::Get())
            {
                FLinearColor* LC = static_cast<FLinearColor*>(ValuePtr);
                return FString::Printf(TEXT("(R=%f, G=%f, B=%f, A=%f)"), LC->R, LC->G, LC->B, LC->A);
            }
            else if (StructProp->Struct == TBaseStructure<FColor>::Get())
            {
                FColor* C = static_cast<FColor*>(ValuePtr);
                return FString::Printf(TEXT("(R=%d, G=%d, B=%d, A=%d)"), C->R, C->G, C->B, C->A);
            }
        }
        else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
        {
            return TEXT("[...]");
        }

        return TEXT("");
    }
};

REGISTER_MCP_COMMAND(FMcpGetAnimNodeDetailsHandler)