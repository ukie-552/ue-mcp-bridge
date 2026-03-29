#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionPanner.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "UObject/PropertyPortFlags.h"
#include "Engine/Texture.h"
#include "Engine/EngineTypes.h"

class FMcpSetMaterialNodePropertyHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_material_node_property"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString MaterialPath;
        if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'material_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NodeName;
        if (!Params->TryGetStringField(TEXT("node_name"), NodeName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'node_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString PropertyName;
        if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'property_name' parameter"), TEXT("MISSING_PARAM"));
        }

        const TSharedPtr<FJsonObject>* PropertyValueObj;
        if (!Params->TryGetObjectField(TEXT("property_value"), PropertyValueObj))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'property_value' parameter"), TEXT("MISSING_PARAM"));
        }

        UMaterial* Material = LoadObject<UMaterial>(nullptr, *MaterialPath);
        if (!Material)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Material not found: %s"), *MaterialPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UMaterialExpression* Expression = FindExpression(Material, NodeName);
        if (!Expression)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Node not found: %s"), *NodeName),
                TEXT("NODE_NOT_FOUND")
            );
        }

        bool bSuccess = SetExpressionProperty(Expression, PropertyName, *PropertyValueObj);

        if (bSuccess)
        {
            Material->MarkPackageDirty();
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("success"), bSuccess);
        Result->SetStringField(TEXT("node_name"), NodeName);
        Result->SetStringField(TEXT("property_name"), PropertyName);

        return FMcpCommandResult::Success(Result);
    }

private:
    UMaterialExpression* FindExpression(UMaterial* Material, const FString& NodeName)
    {
        if (!Material)
        {
            return nullptr;
        }

        const FMaterialExpressionCollection& Collection = Material->GetExpressionCollection();
        for (UMaterialExpression* Expression : Collection.Expressions)
        {
            if (Expression && Expression->GetName() == NodeName)
            {
                return Expression;
            }
        }

        return nullptr;
    }

    bool SetExpressionProperty(UMaterialExpression* Expression, const FString& PropertyName, const TSharedPtr<FJsonObject>& ValueObj)
    {
        if (!Expression)
        {
            return false;
        }

        if (UMaterialExpressionConstant* ConstExpr = Cast<UMaterialExpressionConstant>(Expression))
        {
            if (PropertyName == TEXT("R") || PropertyName == TEXT("Value"))
            {
                double Value = 0;
                if (ValueObj->TryGetNumberField(TEXT("value"), Value))
                {
                    ConstExpr->R = static_cast<float>(Value);
                    return true;
                }
            }
        }
        else if (UMaterialExpressionConstant2Vector* Const2Expr = Cast<UMaterialExpressionConstant2Vector>(Expression))
        {
            if (PropertyName == TEXT("R") || PropertyName == TEXT("X"))
            {
                double Value = 0;
                if (ValueObj->TryGetNumberField(TEXT("value"), Value))
                {
                    Const2Expr->R = static_cast<float>(Value);
                    return true;
                }
            }
            else if (PropertyName == TEXT("G") || PropertyName == TEXT("Y"))
            {
                double Value = 0;
                if (ValueObj->TryGetNumberField(TEXT("value"), Value))
                {
                    Const2Expr->G = static_cast<float>(Value);
                    return true;
                }
            }
        }
        else if (UMaterialExpressionConstant3Vector* Const3Expr = Cast<UMaterialExpressionConstant3Vector>(Expression))
        {
            if (PropertyName == TEXT("Constant") || PropertyName == TEXT("Color"))
            {
                FLinearColor Color;
                if (GetColorFromJson(ValueObj, Color))
                {
                    Const3Expr->Constant = Color;
                    return true;
                }
            }
        }
        else if (UMaterialExpressionConstant4Vector* Const4Expr = Cast<UMaterialExpressionConstant4Vector>(Expression))
        {
            if (PropertyName == TEXT("Constant") || PropertyName == TEXT("Color"))
            {
                FLinearColor Color;
                if (GetColorFromJson(ValueObj, Color))
                {
                    Const4Expr->Constant = Color;
                    return true;
                }
            }
        }
        else if (UMaterialExpressionTextureSample* TexExpr = Cast<UMaterialExpressionTextureSample>(Expression))
        {
            if (PropertyName == TEXT("Texture"))
            {
                FString TexturePath;
                if (ValueObj->TryGetStringField(TEXT("asset_path"), TexturePath))
                {
                    UTexture* Texture = LoadObject<UTexture>(nullptr, *TexturePath);
                    if (Texture)
                    {
                        TexExpr->Texture = Texture;
                        return true;
                    }
                }
            }
            else if (PropertyName == TEXT("SamplerSource"))
            {
                FString Source;
                if (ValueObj->TryGetStringField(TEXT("value"), Source))
                {
                    if (Source == TEXT("Shared") || Source == TEXT("SharedWrap"))
                    {
                        TexExpr->SamplerSource = SSM_Wrap_WorldGroupSettings;
                    }
                    else
                    {
                        TexExpr->SamplerSource = SSM_FromTextureAsset;
                    }
                    return true;
                }
            }
        }
        else if (UMaterialExpressionScalarParameter* ScalarParam = Cast<UMaterialExpressionScalarParameter>(Expression))
        {
            if (PropertyName == TEXT("DefaultValue") || PropertyName == TEXT("Value"))
            {
                double Value = 0;
                if (ValueObj->TryGetNumberField(TEXT("value"), Value))
                {
                    ScalarParam->DefaultValue = static_cast<float>(Value);
                    return true;
                }
            }
            else if (PropertyName == TEXT("ParameterName"))
            {
                FString Value;
                if (ValueObj->TryGetStringField(TEXT("value"), Value))
                {
                    ScalarParam->ParameterName = *Value;
                    return true;
                }
            }
        }
        else if (UMaterialExpressionVectorParameter* VecParam = Cast<UMaterialExpressionVectorParameter>(Expression))
        {
            if (PropertyName == TEXT("DefaultValue") || PropertyName == TEXT("Color"))
            {
                FLinearColor Color;
                if (GetColorFromJson(ValueObj, Color))
                {
                    VecParam->DefaultValue = Color;
                    return true;
                }
            }
            else if (PropertyName == TEXT("ParameterName"))
            {
                FString Value;
                if (ValueObj->TryGetStringField(TEXT("value"), Value))
                {
                    VecParam->ParameterName = *Value;
                    return true;
                }
            }
        }
        else if (UMaterialExpressionTextureCoordinate* TexCoord = Cast<UMaterialExpressionTextureCoordinate>(Expression))
        {
            if (PropertyName == TEXT("UTiling"))
            {
                double Value = 1;
                if (ValueObj->TryGetNumberField(TEXT("value"), Value))
                {
                    TexCoord->UTiling = static_cast<float>(Value);
                    return true;
                }
            }
            else if (PropertyName == TEXT("VTiling"))
            {
                double Value = 1;
                if (ValueObj->TryGetNumberField(TEXT("value"), Value))
                {
                    TexCoord->VTiling = static_cast<float>(Value);
                    return true;
                }
            }
            else if (PropertyName == TEXT("CoordinateIndex"))
            {
                int32 Value = 0;
                if (ValueObj->TryGetNumberField(TEXT("value"), Value))
                {
                    TexCoord->CoordinateIndex = Value;
                    return true;
                }
            }
        }
        else if (UMaterialExpressionPanner* Panner = Cast<UMaterialExpressionPanner>(Expression))
        {
            if (PropertyName == TEXT("SpeedX"))
            {
                double Value = 0;
                if (ValueObj->TryGetNumberField(TEXT("value"), Value))
                {
                    Panner->SpeedX = static_cast<float>(Value);
                    return true;
                }
            }
            else if (PropertyName == TEXT("SpeedY"))
            {
                double Value = 0;
                if (ValueObj->TryGetNumberField(TEXT("value"), Value))
                {
                    Panner->SpeedY = static_cast<float>(Value);
                    return true;
                }
            }
        }
        else if (UMaterialExpressionComponentMask* Mask = Cast<UMaterialExpressionComponentMask>(Expression))
        {
            if (PropertyName == TEXT("R"))
            {
                bool Value = true;
                if (ValueObj->TryGetBoolField(TEXT("value"), Value))
                {
                    Mask->R = Value;
                    return true;
                }
            }
            else if (PropertyName == TEXT("G"))
            {
                bool Value = true;
                if (ValueObj->TryGetBoolField(TEXT("value"), Value))
                {
                    Mask->G = Value;
                    return true;
                }
            }
            else if (PropertyName == TEXT("B"))
            {
                bool Value = true;
                if (ValueObj->TryGetBoolField(TEXT("value"), Value))
                {
                    Mask->B = Value;
                    return true;
                }
            }
            else if (PropertyName == TEXT("A"))
            {
                bool Value = true;
                if (ValueObj->TryGetBoolField(TEXT("value"), Value))
                {
                    Mask->A = Value;
                    return true;
                }
            }
        }

        UClass* ExprClass = Expression->GetClass();
        FProperty* Property = ExprClass->FindPropertyByName(*PropertyName);

        if (Property)
        {
            void* PropertyAddr = Property->ContainerPtrToValuePtr<void>(Expression);
            return SetGenericProperty(Property, PropertyAddr, ValueObj);
        }

        return false;
    }

    bool SetGenericProperty(FProperty* Property, void* PropertyAddr, const TSharedPtr<FJsonObject>& ValueObj)
    {
        if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
        {
            FString Value;
            if (ValueObj->TryGetStringField(TEXT("value"), Value))
            {
                StrProp->SetPropertyValue(PropertyAddr, Value);
                return true;
            }
        }
        else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
        {
            FString Value;
            if (ValueObj->TryGetStringField(TEXT("value"), Value))
            {
                NameProp->SetPropertyValue(PropertyAddr, *Value);
                return true;
            }
        }
        else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
        {
            bool Value;
            if (ValueObj->TryGetBoolField(TEXT("value"), Value))
            {
                BoolProp->SetPropertyValue(PropertyAddr, Value);
                return true;
            }
        }
        else if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
        {
            int32 Value;
            if (ValueObj->TryGetNumberField(TEXT("value"), Value))
            {
                IntProp->SetPropertyValue(PropertyAddr, Value);
                return true;
            }
        }
        else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
        {
            double Value;
            if (ValueObj->TryGetNumberField(TEXT("value"), Value))
            {
                FloatProp->SetPropertyValue(PropertyAddr, static_cast<float>(Value));
                return true;
            }
        }
        else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Property))
        {
            double Value;
            if (ValueObj->TryGetNumberField(TEXT("value"), Value))
            {
                DoubleProp->SetPropertyValue(PropertyAddr, Value);
                return true;
            }
        }

        return false;
    }

    bool GetColorFromJson(const TSharedPtr<FJsonObject>& JsonObj, FLinearColor& OutColor)
    {
        double R = 1, G = 1, B = 1, A = 1;
        JsonObj->TryGetNumberField(TEXT("r"), R);
        JsonObj->TryGetNumberField(TEXT("g"), G);
        JsonObj->TryGetNumberField(TEXT("b"), B);
        JsonObj->TryGetNumberField(TEXT("a"), A);
        OutColor = FLinearColor(R, G, B, A);
        return true;
    }
};

REGISTER_MCP_COMMAND(FMcpSetMaterialNodePropertyHandler)
