#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "MaterialShared.h"

class FMcpConnectMaterialPinsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("connect_material_pins"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString MaterialPath;
        if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'material_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString SourceNodeName;
        if (!Params->TryGetStringField(TEXT("source_node"), SourceNodeName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'source_node' parameter"), TEXT("MISSING_PARAM"));
        }

        FString SourceOutputName;
        if (!Params->TryGetStringField(TEXT("source_output"), SourceOutputName))
        {
            SourceOutputName = TEXT("");
        }

        FString TargetNodeName;
        if (!Params->TryGetStringField(TEXT("target_node"), TargetNodeName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'target_node' parameter"), TEXT("MISSING_PARAM"));
        }

        FString TargetInputName;
        if (!Params->TryGetStringField(TEXT("target_input"), TargetInputName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'target_input' parameter"), TEXT("MISSING_PARAM"));
        }

        UMaterial* Material = LoadObject<UMaterial>(nullptr, *MaterialPath);
        if (!Material)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Material not found: %s"), *MaterialPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UMaterialExpression* SourceExpression = FindExpression(Material, SourceNodeName);
        if (!SourceExpression)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Source node not found: %s"), *SourceNodeName),
                TEXT("SOURCE_NODE_NOT_FOUND")
            );
        }

        UMaterialExpression* TargetExpression = FindExpression(Material, TargetNodeName);
        if (!TargetExpression)
        {
            if (TargetNodeName == TEXT("BaseColor") || 
                TargetNodeName == TEXT("Metallic") || 
                TargetNodeName == TEXT("Specular") ||
                TargetNodeName == TEXT("Roughness") ||
                TargetNodeName == TEXT("EmissiveColor") ||
                TargetNodeName == TEXT("Normal") ||
                TargetNodeName == TEXT("WorldPositionOffset") ||
                TargetNodeName == TEXT("AmbientOcclusion"))
            {
                return ConnectToMaterialOutput(Material, SourceExpression, SourceOutputName, TargetNodeName);
            }

            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Target node not found: %s"), *TargetNodeName),
                TEXT("TARGET_NODE_NOT_FOUND")
            );
        }

        FExpressionInput* Input = FindInput(TargetExpression, TargetInputName);
        if (!Input)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Target input not found: %s"), *TargetInputName),
                TEXT("INPUT_NOT_FOUND")
            );
        }

        int32 OutputIndex = FindOutputIndex(SourceExpression, SourceOutputName);

        Input->Expression = SourceExpression;
        Input->OutputIndex = OutputIndex;

        Material->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("connected"), true);
        Result->SetStringField(TEXT("source_node"), SourceNodeName);
        Result->SetStringField(TEXT("source_output"), SourceOutputName);
        Result->SetStringField(TEXT("target_node"), TargetNodeName);
        Result->SetStringField(TEXT("target_input"), TargetInputName);

        return FMcpCommandResult::Success(Result);
    }

private:
    UMaterialExpression* FindExpression(UMaterial* Material, const FString& NodeName)
    {
        if (!Material)
        {
            return nullptr;
        }

        TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions = Material->GetExpressions();
        for (UMaterialExpression* Expression : Expressions)
        {
            if (Expression && Expression->GetName() == NodeName)
            {
                return Expression;
            }
        }

        return nullptr;
    }

    FExpressionInput* FindInput(UMaterialExpression* Expression, const FString& InputName)
    {
        if (!Expression)
        {
            return nullptr;
        }

        int32 InputIndex = 0;
        if (InputName.IsNumeric())
        {
            InputIndex = FCString::Atoi(*InputName);
        }

        if (InputIndex >= 0)
        {
            return Expression->GetInput(InputIndex);
        }

        return nullptr;
    }

    int32 FindOutputIndex(UMaterialExpression* Expression, const FString& OutputName)
    {
        if (!Expression || OutputName.IsEmpty())
        {
            return 0;
        }

        TArray<FExpressionOutput>& Outputs = Expression->GetOutputs();
        for (int32 i = 0; i < Outputs.Num(); ++i)
        {
            if (Outputs[i].OutputName.ToString() == OutputName)
            {
                return i;
            }
        }

        return 0;
    }

    FMcpCommandResult ConnectToMaterialOutput(UMaterial* Material, UMaterialExpression* SourceExpression, const FString& OutputName, const FString& MaterialProperty)
    {
        if (!Material || !SourceExpression)
        {
            return FMcpCommandResult::Failure(TEXT("Invalid material or expression"), TEXT("INVALID_PARAM"));
        }

        static TMap<FString, EMaterialProperty> PropertyMap = {
            {TEXT("BaseColor"), MP_BaseColor},
            {TEXT("Metallic"), MP_Metallic},
            {TEXT("Specular"), MP_Specular},
            {TEXT("Roughness"), MP_Roughness},
            {TEXT("EmissiveColor"), MP_EmissiveColor},
            {TEXT("Normal"), MP_Normal},
            {TEXT("WorldPositionOffset"), MP_WorldPositionOffset},
            {TEXT("AmbientOcclusion"), MP_AmbientOcclusion},
            {TEXT("Opacity"), MP_Opacity},
            {TEXT("OpacityMask"), MP_OpacityMask},
        };

        EMaterialProperty* Property = PropertyMap.Find(MaterialProperty);
        if (!Property)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Unknown material property: %s"), *MaterialProperty),
                TEXT("UNKNOWN_PROPERTY")
            );
        }

        int32 OutputIndex = FindOutputIndex(SourceExpression, OutputName);

        FExpressionInput* ExpressionInput = Material->GetExpressionInputForProperty(*Property);
        if (ExpressionInput)
        {
            ExpressionInput->Expression = SourceExpression;
            ExpressionInput->OutputIndex = OutputIndex;
        }

        Material->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("connected"), true);
        Result->SetStringField(TEXT("source_node"), SourceExpression->GetName());
        Result->SetStringField(TEXT("material_property"), MaterialProperty);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpConnectMaterialPinsHandler)
