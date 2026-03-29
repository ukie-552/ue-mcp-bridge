#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameters.h"

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
        Properties->SetBoolField(TEXT("is_translucent"), Material->IsTranslucentBlendMode());
        Properties->SetBoolField(TEXT("is_masked"), Material->IsMasked());

        FString BlendMode;
        switch (Material->GetBlendMode())
        {
            case BLEND_Opaque: BlendMode = TEXT("Opaque"); break;
            case BLEND_Masked: BlendMode = TEXT("Masked"); break;
            case BLEND_Translucent: BlendMode = TEXT("Translucent"); break;
            case BLEND_Additive: BlendMode = TEXT("Additive"); break;
            case BLEND_Modulate: BlendMode = TEXT("Modulate"); break;
            case BLEND_AlphaComposite: BlendMode = TEXT("AlphaComposite"); break;
            default: BlendMode = TEXT("Unknown"); break;
        }
        Properties->SetStringField(TEXT("blend_mode"), BlendMode);

        FString ShadingModel;
        switch (Material->GetShadingModels())
        {
            case MSM_Unlit: ShadingModel = TEXT("Unlit"); break;
            case MSM_DefaultLit: ShadingModel = TEXT("DefaultLit"); break;
            case MSM_Subsurface: ShadingModel = TEXT("Subsurface"); break;
            case MSM_PreintegratedSkin: ShadingModel = TEXT("PreintegratedSkin"); break;
            case MSM_ClearCoat: ShadingModel = TEXT("ClearCoat"); break;
            case MSM_SubsurfaceProfile: ShadingModel = TEXT("SubsurfaceProfile"); break;
            case MSM_TwoSidedFoliage: ShadingModel = TEXT("TwoSidedFoliage"); break;
            case MSM_Hair: ShadingModel = TEXT("Hair"); break;
            case MSM_Cloth: ShadingModel = TEXT("Cloth"); break;
            case MSM_Eye: ShadingModel = TEXT("Eye"); break;
            case MSM_SingleLayerWater: ShadingModel = TEXT("SingleLayerWater"); break;
            case MSM_ThinTranslucent: ShadingModel = TEXT("ThinTranslucent"); break;
            default: ShadingModel = TEXT("Unknown"); break;
        }
        Properties->SetStringField(TEXT("shading_model"), ShadingModel);

        FString MaterialDomain;
        switch (Material->GetMaterialDomain())
        {
            case MD_Surface: MaterialDomain = TEXT("Surface"); break;
            case MD_DeferredDecal: MaterialDomain = TEXT("DeferredDecal"); break;
            case MD_LightFunction: MaterialDomain = TEXT("LightFunction"); break;
            case MD_Volume: MaterialDomain = TEXT("Volume"); break;
            case MD_PostProcess: MaterialDomain = TEXT("PostProcess"); break;
            case MD_UserInterface: MaterialDomain = TEXT("UserInterface"); break;
            default: MaterialDomain = TEXT("Unknown"); break;
        }
        Properties->SetStringField(TEXT("material_domain"), MaterialDomain);

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

        ShaderInfo->SetBoolField(TEXT("is_compiled"), Material->IsMaterialCompiled());

        TArray<FString> UsedTextures;
        Material->GetUsedTextures(UsedTextures, EMaterialQualityLevel::Num, ERHIFeatureLevel::Num, true);

        TArray<TSharedPtr<FJsonValue>> TexturesArray;
        for (const FString& TexturePath : UsedTextures)
        {
            TexturesArray.Add(MakeShareable(new FJsonValueString(TexturePath)));
        }
        ShaderInfo->SetArrayField(TEXT("used_textures"), TexturesArray);
        ShaderInfo->SetNumberField(TEXT("texture_count"), UsedTextures.Num());

        OutResult->SetObjectField(TEXT("shader"), ShaderInfo);
    }
};

REGISTER_MCP_COMMAND(FMcpGetMaterialInfoHandler)
