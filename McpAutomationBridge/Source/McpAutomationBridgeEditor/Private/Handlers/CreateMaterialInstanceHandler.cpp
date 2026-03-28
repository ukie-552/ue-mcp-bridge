#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/MaterialInstanceConstant.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"

class FMcpCreateMaterialInstanceHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_material_instance"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString InstancePath;
        if (!Params->TryGetStringField(TEXT("instance_path"), InstancePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'instance_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString InstanceName;
        if (!Params->TryGetStringField(TEXT("instance_name"), InstanceName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'instance_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ParentMaterialPath;
        Params->TryGetStringField(TEXT("parent_material"), ParentMaterialPath);

        FString FullPath = FString::Printf(TEXT("%s/%s"), *InstancePath, *InstanceName);

        UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
        if (!Factory)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to create material instance factory"), TEXT("FACTORY_ERROR"));
        }

        if (!ParentMaterialPath.IsEmpty())
        {
            UMaterial* ParentMaterial = LoadObject<UMaterial>(nullptr, *ParentMaterialPath);
            if (ParentMaterial)
            {
                Factory->InitialParent = ParentMaterial;
            }
        }

        UMaterialInstanceConstant* NewInstance = Cast<UMaterialInstanceConstant>(Factory->FactoryCreateNew(
            UMaterialInstanceConstant::StaticClass(),
            GetTransientPackage(),
            *InstanceName,
            RF_Public | RF_Standalone,
            nullptr,
            GWarn
        ));

        if (!NewInstance)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create material instance: %s"), *InstanceName),
                TEXT("INSTANCE_CREATION_FAILED")
            );
        }

        FAssetRegistryModule::AssetCreated(NewInstance);
        NewInstance->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("instance_name"), InstanceName);
        Result->SetStringField(TEXT("instance_path"), FullPath);
        
        if (NewInstance->Parent)
        {
            Result->SetStringField(TEXT("parent_material"), NewInstance->Parent->GetPathName());
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateMaterialInstanceHandler)
