#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameters.h"
#include "Engine/Texture.h"

class FMcpGetMaterialInfoHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_material_info"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString MaterialPath;
        if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'material_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bIncludeExpressions = false;
        Params->TryGetBoolField(TEXT("include_expressions"), bIncludeExpressions);

        bool bIncludeParameters = true;
        Params->TryGetBoolField(TEXT("include_parameters"), bIncludeParameters);

        UMaterialInterface* MaterialInterface = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
        if (!MaterialInterface)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Material not found: %s"), *MaterialPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);

        CollectBasicInfo(MaterialInterface, Result);
        CollectMaterialProperties(MaterialInterface, Result);

        if (bIncludeParameters)
        {
            CollectParameters(MaterialInterface, Result);
        }

        if (bIncludeExpressions)
        {
            CollectExpressions(MaterialInterface, Result);
        }

        UMaterial* Material = Cast<UMaterial>(MaterialInterface);
        if (Material)
        {
            CollectShaderInfo(Material, Result);
        }

        return FMcpCommandResult::Success(Result);
    }

private:
    void CollectBasicInfo(UMaterialInterface* Material, TSharedPtr<FJsonObject>& OutResult)
    {
        OutResult->SetStringField(TEXT("name"), Material->GetName());
        OutResult->SetStringField(TEXT("path"), Material->GetPathName());
        OutResult->SetStringField(TEXT("class"), Material->GetClass()->GetName());

        UMaterial* BaseMaterial = Cast<UMaterial>(Material);
        OutResult->SetBoolField(TEXT("is_material_instance"), BaseMaterial == nullptr);

        if (UMaterialInstance* Instance = Cast<UMaterialInstance>(Material))
        {
            if (Instance->Parent)
            {
                OutResult->SetStringField(TEXT("parent_material"), Instance->Parent->GetPathName());
            }
        }
    }

    void CollectMaterialProperties(UMaterialInterface* Material, TSharedPtr<FJsonObject>& OutResult)
    {
        TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject);

        Properties->SetBoolField(TEXT("is_two_sided"), Material->IsTwoSided());
        
        EBlendMode BlendMode = Material->GetBlendMode();
        bool bIsTranslucent = (BlendMode == BLEND_Translucent || BlendMode == BLEND_Additive || BlendMode == BLEND_Modulate);
        Properties->SetBoolField(TEXT("is_translucent"), bIsTranslucent);
        Properties->SetBoolField(TEXT("is_masked"), Material->IsMasked());

        FString BlendModeStr;
        switch (BlendMode)
        {
            case BLEND_Opaque: BlendModeStr = TEXT("Opaque"); break;
            case BLEND_Masked: BlendModeStr = TEXT("Masked"); break;
            case BLEND_Translucent: BlendModeStr = TEXT("Translucent"); break;
            case BLEND_Additive: BlendModeStr = TEXT("Additive"); break;
            case BLEND_Modulate: BlendModeStr = TEXT("Modulate"); break;
            case BLEND_AlphaComposite: BlendModeStr = TEXT("AlphaComposite"); break;
            default: BlendModeStr = TEXT("Unknown"); break;
        }
        Properties->SetStringField(TEXT("blend_mode"), BlendModeStr);

        FMaterialShadingModelField ShadingModels = Material->GetShadingModels();
        TArray<FString> ShadingModelNames;
        
        if (ShadingModels.IsUnlit()) ShadingModelNames.Add(TEXT("Unlit"));
        if (ShadingModels.HasShadingModel(MSM_DefaultLit)) ShadingModelNames.Add(TEXT("DefaultLit"));
        if (ShadingModels.HasShadingModel(MSM_Subsurface)) ShadingModelNames.Add(TEXT("Subsurface"));
        if (ShadingModels.HasShadingModel(MSM_PreintegratedSkin)) ShadingModelNames.Add(TEXT("PreintegratedSkin"));
        if (ShadingModels.HasShadingModel(MSM_ClearCoat)) ShadingModelNames.Add(TEXT("ClearCoat"));
        if (ShadingModels.HasShadingModel(MSM_SubsurfaceProfile)) ShadingModelNames.Add(TEXT("SubsurfaceProfile"));
        if (ShadingModels.HasShadingModel(MSM_TwoSidedFoliage)) ShadingModelNames.Add(TEXT("TwoSidedFoliage"));
        if (ShadingModels.HasShadingModel(MSM_Hair)) ShadingModelNames.Add(TEXT("Hair"));
        if (ShadingModels.HasShadingModel(MSM_Cloth)) ShadingModelNames.Add(TEXT("Cloth"));
        if (ShadingModels.HasShadingModel(MSM_Eye)) ShadingModelNames.Add(TEXT("Eye"));
        if (ShadingModels.HasShadingModel(MSM_SingleLayerWater)) ShadingModelNames.Add(TEXT("SingleLayerWater"));
        if (ShadingModels.HasShadingModel(MSM_ThinTranslucent)) ShadingModelNames.Add(TEXT("ThinTranslucent"));
        
        FString ShadingModelStr = ShadingModelNames.Num() > 0 ? FString::Join(ShadingModelNames, TEXT(", ")) : TEXT("Unknown");
        Properties->SetStringField(TEXT("shading_model"), ShadingModelStr);

        OutResult->SetObjectField(TEXT("properties"), Properties);
    }

    void CollectParameters(UMaterialInterface* Material, TSharedPtr<FJsonObject>& OutResult)
    {
        TArray<TSharedPtr<FJsonValue>> ScalarParams;
        TArray<TSharedPtr<FJsonValue>> VectorParams;
        TArray<TSharedPtr<FJsonValue>> TextureParams;

        TArray<FMaterialParameterInfo> ScalarParameterInfo;
        TArray<FGuid> ScalarParameterIds;
        Material->GetAllScalarParameterInfo(ScalarParameterInfo, ScalarParameterIds);

        for (int32 i = 0; i < ScalarParameterInfo.Num(); ++i)
        {
            const FMaterialParameterInfo& ParamInfo = ScalarParameterInfo[i];
            const FGuid& ParamId = ScalarParameterIds[i];

            float Value = 0.0f;
            if (Material->GetScalarParameterValue(ParamInfo, Value))
            {
                TSharedPtr<FJsonObject> Param = MakeShareable(new FJsonObject);
                Param->SetStringField(TEXT("name"), ParamInfo.Name.ToString());
                Param->SetStringField(TEXT("guid"), ParamId.ToString());
                Param->SetNumberField(TEXT("value"), Value);
                ScalarParams.Add(MakeShareable(new FJsonValueObject(Param)));
            }
        }

        TArray<FMaterialParameterInfo> VectorParameterInfo;
        TArray<FGuid> VectorParameterIds;
        Material->GetAllVectorParameterInfo(VectorParameterInfo, VectorParameterIds);

        for (int32 i = 0; i < VectorParameterInfo.Num(); ++i)
        {
            const FMaterialParameterInfo& ParamInfo = VectorParameterInfo[i];
            const FGuid& ParamId = VectorParameterIds[i];

            FLinearColor Value;
            if (Material->GetVectorParameterValue(ParamInfo, Value))
            {
                TSharedPtr<FJsonObject> Param = MakeShareable(new FJsonObject);
                Param->SetStringField(TEXT("name"), ParamInfo.Name.ToString());
                Param->SetStringField(TEXT("guid"), ParamId.ToString());
                
                TSharedPtr<FJsonObject> ColorValue = MakeShareable(new FJsonObject);
                ColorValue->SetNumberField(TEXT("r"), Value.R);
                ColorValue->SetNumberField(TEXT("g"), Value.G);
                ColorValue->SetNumberField(TEXT("b"), Value.B);
                ColorValue->SetNumberField(TEXT("a"), Value.A);
                Param->SetObjectField(TEXT("value"), ColorValue);
                
                VectorParams.Add(MakeShareable(new FJsonValueObject(Param)));
            }
        }

        TArray<FMaterialParameterInfo> TextureParameterInfo;
        TArray<FGuid> TextureParameterIds;
        Material->GetAllTextureParameterInfo(TextureParameterInfo, TextureParameterIds);

        for (int32 i = 0; i < TextureParameterInfo.Num(); ++i)
        {
            const FMaterialParameterInfo& ParamInfo = TextureParameterInfo[i];
            const FGuid& ParamId = TextureParameterIds[i];

            UTexture* Value = nullptr;
            if (Material->GetTextureParameterValue(ParamInfo, Value))
            {
                TSharedPtr<FJsonObject> Param = MakeShareable(new FJsonObject);
                Param->SetStringField(TEXT("name"), ParamInfo.Name.ToString());
                Param->SetStringField(TEXT("guid"), ParamId.ToString());
                if (Value)
                {
                    Param->SetStringField(TEXT("texture_path"), Value->GetPathName());
                }
                TextureParams.Add(MakeShareable(new FJsonValueObject(Param)));
            }
        }

        TSharedPtr<FJsonObject> Parameters = MakeShareable(new FJsonObject);
        Parameters->SetArrayField(TEXT("scalar"), ScalarParams);
        Parameters->SetArrayField(TEXT("vector"), VectorParams);
        Parameters->SetArrayField(TEXT("texture"), TextureParams);
        Parameters->SetNumberField(TEXT("total_count"), ScalarParams.Num() + VectorParams.Num() + TextureParams.Num());

        OutResult->SetObjectField(TEXT("parameters"), Parameters);
    }

    void CollectExpressions(UMaterialInterface* Material, TSharedPtr<FJsonObject>& OutResult)
    {
        UMaterial* BaseMaterial = Cast<UMaterial>(Material);
        if (!BaseMaterial)
        {
            return;
        }

        TArray<TSharedPtr<FJsonValue>> Expressions;

        for (UMaterialExpression* Expression : BaseMaterial->GetExpressions())
        {
            if (!Expression)
            {
                continue;
            }

            TSharedPtr<FJsonObject> Expr = MakeShareable(new FJsonObject);
            Expr->SetStringField(TEXT("type"), Expression->GetClass()->GetName());
            Expr->SetStringField(TEXT("name"), Expression->GetName());
            
            FString Desc = Expression->GetDescription();
            if (!Desc.IsEmpty())
            {
                Expr->SetStringField(TEXT("description"), Desc);
            }

            Expressions.Add(MakeShareable(new FJsonValueObject(Expr)));
        }

        OutResult->SetArrayField(TEXT("expressions"), Expressions);
        OutResult->SetNumberField(TEXT("expression_count"), Expressions.Num());
    }

    void CollectShaderInfo(UMaterial* Material, TSharedPtr<FJsonObject>& OutResult)
    {
        TSharedPtr<FJsonObject> ShaderInfo = MakeShareable(new FJsonObject);

        ShaderInfo->SetBoolField(TEXT("is_compiled"), true);

        TArray<UTexture*> UsedTextures;
        Material->GetUsedTextures(UsedTextures);

        TArray<TSharedPtr<FJsonValue>> TexturesArray;
        for (UTexture* Texture : UsedTextures)
        {
            if (Texture)
            {
                TexturesArray.Add(MakeShareable(new FJsonValueString(Texture->GetPathName())));
            }
        }
        ShaderInfo->SetArrayField(TEXT("used_textures"), TexturesArray);
        ShaderInfo->SetNumberField(TEXT("texture_count"), UsedTextures.Num());

        OutResult->SetObjectField(TEXT("shader"), ShaderInfo);
    }
};

REGISTER_MCP_COMMAND(FMcpGetMaterialInfoHandler)
