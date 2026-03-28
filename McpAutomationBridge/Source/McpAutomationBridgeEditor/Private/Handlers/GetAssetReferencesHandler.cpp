#include "CoreMinimal.h"
#include "McpCommand.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "EditorAssetLibrary.h"

class FMcpGetAssetReferencesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_asset_references"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AssetPath;
        if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'asset_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bIncludeHard = true;
        Params->TryGetBoolField(TEXT("include_hard"), bIncludeHard);

        bool bIncludeSoft = true;
        Params->TryGetBoolField(TEXT("include_soft"), bIncludeSoft);

        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
        IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

        TArray<FName> HardReferences;
        TArray<FName> SoftReferences;

        if (bIncludeHard)
        {
            AssetRegistry.GetReferencers(*AssetPath, HardReferences, UE::AssetRegistry::EDependencyCategory::Package, UE::AssetRegistry::EDependencyQuery::Hard);
        }

        if (bIncludeSoft)
        {
            AssetRegistry.GetReferencers(*AssetPath, SoftReferences, UE::AssetRegistry::EDependencyCategory::Package, UE::AssetRegistry::EDependencyQuery::Soft);
        }

        TArray<TSharedPtr<FJsonValue>> HardRefsArray;
        for (const FName& Ref : HardReferences)
        {
            HardRefsArray.Add(MakeShareable(new FJsonValueString(Ref.ToString())));
        }

        TArray<TSharedPtr<FJsonValue>> SoftRefsArray;
        for (const FName& Ref : SoftReferences)
        {
            SoftRefsArray.Add(MakeShareable(new FJsonValueString(Ref.ToString())));
        }

        TArray<FName> HardDependencies;
        TArray<FName> SoftDependencies;

        if (bIncludeHard)
        {
            AssetRegistry.GetDependencies(*AssetPath, HardDependencies, UE::AssetRegistry::EDependencyCategory::Package, UE::AssetRegistry::EDependencyQuery::Hard);
        }

        if (bIncludeSoft)
        {
            AssetRegistry.GetDependencies(*AssetPath, SoftDependencies, UE::AssetRegistry::EDependencyCategory::Package, UE::AssetRegistry::EDependencyQuery::Soft);
        }

        TArray<TSharedPtr<FJsonValue>> HardDepsArray;
        for (const FName& Dep : HardDependencies)
        {
            HardDepsArray.Add(MakeShareable(new FJsonValueString(Dep.ToString())));
        }

        TArray<TSharedPtr<FJsonValue>> SoftDepsArray;
        for (const FName& Dep : SoftDependencies)
        {
            SoftDepsArray.Add(MakeShareable(new FJsonValueString(Dep.ToString())));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("asset_path"), AssetPath);
        Result->SetArrayField(TEXT("hard_referencers"), HardRefsArray);
        Result->SetArrayField(TEXT("soft_referencers"), SoftRefsArray);
        Result->SetArrayField(TEXT("hard_dependencies"), HardDepsArray);
        Result->SetArrayField(TEXT("soft_dependencies"), SoftDepsArray);
        Result->SetNumberField(TEXT("hard_referencer_count"), HardRefsArray.Num());
        Result->SetNumberField(TEXT("soft_referencer_count"), SoftRefsArray.Num());
        Result->SetNumberField(TEXT("hard_dependency_count"), HardDepsArray.Num());
        Result->SetNumberField(TEXT("soft_dependency_count"), SoftDepsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAssetReferencesHandler)
