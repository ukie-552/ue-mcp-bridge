#include "CoreMinimal.h"
#include "McpCommand.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "EditorAssetLibrary.h"
#include "Factories/Factory.h"

class FMcpFindAssetsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("find_assets"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString PackagePath = TEXT("/Game");
        Params->TryGetStringField(TEXT("package_path"), PackagePath);

        FString ClassNameFilter;
        Params->TryGetStringField(TEXT("class_name"), ClassNameFilter);

        FString NameContains;
        Params->TryGetStringField(TEXT("name_contains"), NameContains);

        bool bRecursive = true;
        Params->TryGetBoolField(TEXT("recursive"), bRecursive);

        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
        IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

        TArray<FAssetData> AssetDataList;
        FARFilter Filter;
        Filter.PackagePaths.Add(*PackagePath);
        Filter.bRecursivePaths = bRecursive;

        if (!ClassNameFilter.IsEmpty())
        {
            UClass* FilterClass = LoadObject<UClass>(nullptr, *ClassNameFilter);
            if (FilterClass)
            {
                Filter.ClassPaths.Add(FilterClass->GetClassPathName());
            }
        }

        AssetRegistry.GetAssets(Filter, AssetDataList);

        TArray<TSharedPtr<FJsonValue>> AssetsArray;

        for (const FAssetData& AssetData : AssetDataList)
        {
            FString AssetName = AssetData.AssetName.ToString();

            if (!NameContains.IsEmpty() && !AssetName.Contains(NameContains))
            {
                continue;
            }

            TSharedPtr<FJsonObject> AssetJson = MakeShareable(new FJsonObject);
            AssetJson->SetStringField(TEXT("asset_name"), AssetName);
            AssetJson->SetStringField(TEXT("package_name"), AssetData.PackageName.ToString());
            AssetJson->SetStringField(TEXT("package_path"), AssetData.PackagePath.ToString());
            AssetJson->SetStringField(TEXT("class_name"), AssetData.AssetClassPath.ToString());
            AssetJson->SetStringField(TEXT("object_path"), AssetData.GetObjectPathString());

            FString AssetPath = AssetData.GetObjectPathString();
            AssetJson->SetStringField(TEXT("asset_path"), AssetPath);

            AssetsArray.Add(MakeShareable(new FJsonValueObject(AssetJson)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("assets"), AssetsArray);
        Result->SetNumberField(TEXT("count"), AssetsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpFindAssetsHandler)
