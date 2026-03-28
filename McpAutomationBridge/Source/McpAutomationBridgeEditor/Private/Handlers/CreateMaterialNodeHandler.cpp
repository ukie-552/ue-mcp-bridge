#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "AssetRegistry/AssetRegistryModule.h"

class FMcpCreateMaterialNodeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_material_node"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString MaterialPath;
        if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'material_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NodeType;
        if (!Params->TryGetStringField(TEXT("node_type"), NodeType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'node_type' parameter"), TEXT("MISSING_PARAM"));
        }

        int32 PosX = 0, PosY = 0;
        const TSharedPtr<FJsonObject>* PositionObj;
        if (Params->TryGetObjectField(TEXT("position"), PositionObj))
        {
            (*PositionObj)->TryGetNumberField(TEXT("x"), PosX);
            (*PositionObj)->TryGetNumberField(TEXT("y"), PosY);
        }

        UMaterial* Material = LoadObject<UMaterial>(nullptr, *MaterialPath);
        if (!Material)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Material not found: %s"), *MaterialPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UMaterialExpression* NewExpression = CreateExpressionByType(Material, NodeType, Params);
        if (!NewExpression)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create material node of type: %s"), *NodeType),
                TEXT("NODE_CREATION_FAILED")
            );
        }

        NewExpression->MaterialExpressionEditorX = PosX;
        NewExpression->MaterialExpressionEditorY = PosY;

        Material->GetExpressionCollection().AddExpression(NewExpression);
        Material->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("node_type"), NodeType);
        Result->SetStringField(TEXT("node_id"), NewExpression->GetName());
        Result->SetNumberField(TEXT("pos_x"), NewExpression->MaterialExpressionEditorX);
        Result->SetNumberField(TEXT("pos_y"), NewExpression->MaterialExpressionEditorY);

        TArray<FExpressionOutput>& Outputs = NewExpression->GetOutputs();
        TArray<TSharedPtr<FJsonValue>> OutputArray;
        for (int32 i = 0; i < Outputs.Num(); ++i)
        {
            TSharedPtr<FJsonObject> OutputObj = MakeShareable(new FJsonObject);
            OutputObj->SetNumberField(TEXT("index"), i);
            OutputObj->SetStringField(TEXT("name"), Outputs[i].OutputName.ToString());
            OutputArray.Add(MakeShareable(new FJsonValueObject(OutputObj)));
        }
        Result->SetArrayField(TEXT("outputs"), OutputArray);

        return FMcpCommandResult::Success(Result);
    }

private:
    UMaterialExpression* CreateExpressionByType(UMaterial* Material, const FString& NodeType, const TSharedPtr<FJsonObject>& Params)
    {
        UClass* ExprClass = FindExpressionClass(NodeType);
        if (!ExprClass)
        {
            return nullptr;
        }

        UMaterialExpression* Expression = NewObject<UMaterialExpression>(Material, ExprClass);
        if (Expression)
        {
            Expression->SetFlags(RF_Transactional);
            ConfigureExpression(Expression, NodeType, Params);
        }

        return Expression;
    }

    UClass* FindExpressionClass(const FString& NodeType)
    {
        static TMap<FString, FString> NodeTypeToClass = {
            {TEXT("Constant"), TEXT("MaterialExpressionConstant")},
            {TEXT("Constant1"), TEXT("MaterialExpressionConstant")},
            {TEXT("Constant2Vector"), TEXT("MaterialExpressionConstant2Vector")},
            {TEXT("Constant2"), TEXT("MaterialExpressionConstant2Vector")},
            {TEXT("Constant3Vector"), TEXT("MaterialExpressionConstant3Vector")},
            {TEXT("Constant3"), TEXT("MaterialExpressionConstant3Vector")},
            {TEXT("Constant4Vector"), TEXT("MaterialExpressionConstant4Vector")},
            {TEXT("Constant4"), TEXT("MaterialExpressionConstant4Vector")},
            {TEXT("TextureSample"), TEXT("MaterialExpressionTextureSample")},
            {TEXT("Texture"), TEXT("MaterialExpressionTextureSample")},
            {TEXT("Multiply"), TEXT("MaterialExpressionMultiply")},
            {TEXT("Add"), TEXT("MaterialExpressionAdd")},
            {TEXT("Subtract"), TEXT("MaterialExpressionSubtract")},
            {TEXT("Divide"), TEXT("MaterialExpressionDivide")},
            {TEXT("Power"), TEXT("MaterialExpressionPower")},
            {TEXT("Lerp"), TEXT("MaterialExpressionLinearInterpolate")},
            {TEXT("Clamp"), TEXT("MaterialExpressionClamp")},
            {TEXT("Saturate"), TEXT("MaterialExpressionSaturate")},
            {TEXT("Abs"), TEXT("MaterialExpressionAbs")},
            {TEXT("Sine"), TEXT("MaterialExpressionSine")},
            {TEXT("Cosine"), TEXT("MaterialExpressionCosine")},
            {TEXT("Time"), TEXT("MaterialExpressionTime")},
            {TEXT("Frac"), TEXT("MaterialExpressionFrac")},
            {TEXT("Floor"), TEXT("MaterialExpressionFloor")},
            {TEXT("Ceil"), TEXT("MaterialExpressionCeil")},
            {TEXT("Round"), TEXT("MaterialExpressionRound")},
            {TEXT("OneMinus"), TEXT("MaterialExpressionOneMinus")},
            {TEXT("WorldPosition"), TEXT("MaterialExpressionWorldPosition")},
            {TEXT("ViewSize"), TEXT("MaterialExpressionViewSize")},
            {TEXT("PixelNormal"), TEXT("MaterialExpressionPixelNormal")},
            {TEXT("VertexNormal"), TEXT("MaterialExpressionVertexNormalWS")},
            {TEXT("CameraVector"), TEXT("MaterialExpressionCameraVector")},
            {TEXT("ReflectionVector"), TEXT("MaterialExpressionReflectionVectorWS")},
            {TEXT("ScalarParameter"), TEXT("MaterialExpressionScalarParameter")},
            {TEXT("VectorParameter"), TEXT("MaterialExpressionVectorParameter")},
            {TEXT("TextureObjectParameter"), TEXT("MaterialExpressionTextureObjectParameter")},
            {TEXT("TextureCoordinate"), TEXT("MaterialExpressionTextureCoordinate")},
            {TEXT("TexCoord"), TEXT("MaterialExpressionTextureCoordinate")},
            {TEXT("Panner"), TEXT("MaterialExpressionPanner")},
            {TEXT("Rotator"), TEXT("MaterialExpressionRotator")},
            {TEXT("AppendVector"), TEXT("MaterialExpressionAppendVector")},
            {TEXT("ComponentMask"), TEXT("MaterialExpressionComponentMask")},
            {TEXT("Desaturation"), TEXT("MaterialExpressionDesaturation")},
        };

        FString ClassName;
        if (NodeTypeToClass.Contains(NodeType))
        {
            ClassName = NodeTypeToClass[NodeType];
        }
        else
        {
            ClassName = NodeType;
        }

        FString FullPath = FString::Printf(TEXT("/Script/Engine.%s"), *ClassName);
        UClass* FoundClass = LoadClass<UMaterialExpression>(nullptr, *FullPath);
        
        if (!FoundClass)
        {
            FullPath = FString::Printf(TEXT("Engine.%s"), *ClassName);
            FoundClass = LoadClass<UMaterialExpression>(nullptr, *FullPath);
        }

        return FoundClass;
    }

    void ConfigureExpression(UMaterialExpression* Expression, const FString& NodeType, const TSharedPtr<FJsonObject>& Params)
    {
        if (!Expression)
        {
            return;
        }

        if (NodeType == TEXT("ScalarParameter"))
        {
            FString ParamName;
            if (Params->TryGetStringField(TEXT("parameter_name"), ParamName))
            {
                FProperty* NameProp = Expression->GetClass()->FindPropertyByName(TEXT("ParameterName"));
                if (NameProp)
                {
                    NameProp->SetValue_InContainer(Expression, &ParamName);
                }
            }
        }
        else if (NodeType == TEXT("VectorParameter"))
        {
            FString ParamName;
            if (Params->TryGetStringField(TEXT("parameter_name"), ParamName))
            {
                FProperty* NameProp = Expression->GetClass()->FindPropertyByName(TEXT("ParameterName"));
                if (NameProp)
                {
                    NameProp->SetValue_InContainer(Expression, &ParamName);
                }
            }
        }
    }
};

REGISTER_MCP_COMMAND(FMcpCreateMaterialNodeHandler)
