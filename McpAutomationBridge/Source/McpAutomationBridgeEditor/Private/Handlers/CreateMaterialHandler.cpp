#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

class FMcpCreateMaterialHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_material"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString MaterialPath;
        if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'material_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString MaterialName;
        if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'material_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString FullPath = FString::Printf(TEXT("%s/%s"), *MaterialPath, *MaterialName);

        FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
        
        UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
            MaterialName,
            MaterialPath,
            UMaterial::StaticClass(),
            nullptr
        );

        if (!NewAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create material: %s"), *MaterialName),
                TEXT("MATERIAL_CREATION_FAILED")
            );
        }

        UMaterial* NewMaterial = Cast<UMaterial>(NewAsset);
        if (NewMaterial)
        {
            FAssetRegistryModule::AssetCreated(NewMaterial);
            NewMaterial->MarkPackageDirty();
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("material_name"), MaterialName);
        Result->SetStringField(TEXT("material_path"), FullPath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateMaterialHandler)
