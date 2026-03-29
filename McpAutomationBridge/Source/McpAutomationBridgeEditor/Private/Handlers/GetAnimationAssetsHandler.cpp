#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"

class FMcpGetAnimationAssetsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_animation_assets"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SkeletonPath;
        Params->TryGetStringField(TEXT("skeleton_path"), SkeletonPath);

        FString AssetType = TEXT("all");
        Params->TryGetStringField(TEXT("asset_type"), AssetType);

        FString SearchPath = TEXT("/Game");
        Params->TryGetStringField(TEXT("search_path"), SearchPath);

        USkeleton* TargetSkeleton = nullptr;
        if (!SkeletonPath.IsEmpty())
        {
            TargetSkeleton = LoadObject<USkeleton>(nullptr, *SkeletonPath);
        }

        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
        IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

        TArray<FAssetData> AssetDataList;
        TArray<FName> AssetPaths = { *SearchPath };

        TArray<TSharedPtr<FJsonValue>> AssetsArray;

        if (AssetType == TEXT("all") || AssetType == TEXT("anim_sequence"))
        {
            AssetRegistry.GetAssetsByClass(UAnimSequence::StaticClass()->GetClassPathName(), AssetDataList, true);
            for (const FAssetData& AssetData : AssetDataList)
            {
                if (TargetSkeleton)
                {
                    UAnimSequence* AnimSeq = Cast<UAnimSequence>(AssetData.GetAsset());
                    if (!AnimSeq || AnimSeq->GetSkeleton() != TargetSkeleton)
                        continue;
                }

                TSharedPtr<FJsonObject> AssetInfo = MakeShareable(new FJsonObject);
                AssetInfo->SetStringField(TEXT("asset_name"), AssetData.AssetName.ToString());
                AssetInfo->SetStringField(TEXT("asset_path"), AssetData.GetObjectPathString());
                AssetInfo->SetStringField(TEXT("asset_type"), TEXT("AnimSequence"));

                AssetsArray.Add(MakeShareable(new FJsonValueObject(AssetInfo)));
            }
        }

        if (AssetType == TEXT("all") || AssetType == TEXT("anim_montage"))
        {
            AssetDataList.Empty();
            AssetRegistry.GetAssetsByClass(UAnimMontage::StaticClass()->GetClassPathName(), AssetDataList, true);
            for (const FAssetData& AssetData : AssetDataList)
            {
                if (TargetSkeleton)
                {
                    UAnimMontage* AnimMontage = Cast<UAnimMontage>(AssetData.GetAsset());
                    if (!AnimMontage || AnimMontage->GetSkeleton() != TargetSkeleton)
                        continue;
                }

                TSharedPtr<FJsonObject> AssetInfo = MakeShareable(new FJsonObject);
                AssetInfo->SetStringField(TEXT("asset_name"), AssetData.AssetName.ToString());
                AssetInfo->SetStringField(TEXT("asset_path"), AssetData.GetObjectPathString());
                AssetInfo->SetStringField(TEXT("asset_type"), TEXT("AnimMontage"));

                AssetsArray.Add(MakeShareable(new FJsonValueObject(AssetInfo)));
            }
        }

        if (AssetType == TEXT("all") || AssetType == TEXT("blend_space"))
        {
            AssetDataList.Empty();
            AssetRegistry.GetAssetsByClass(UBlendSpace::StaticClass()->GetClassPathName(), AssetDataList, true);
            for (const FAssetData& AssetData : AssetDataList)
            {
                if (TargetSkeleton)
                {
                    UBlendSpace* BlendSpace = Cast<UBlendSpace>(AssetData.GetAsset());
                    if (!BlendSpace || BlendSpace->GetSkeleton() != TargetSkeleton)
                        continue;
                }

                TSharedPtr<FJsonObject> AssetInfo = MakeShareable(new FJsonObject);
                AssetInfo->SetStringField(TEXT("asset_name"), AssetData.AssetName.ToString());
                AssetInfo->SetStringField(TEXT("asset_path"), AssetData.GetObjectPathString());
                AssetInfo->SetStringField(TEXT("asset_type"), TEXT("BlendSpace"));

                AssetsArray.Add(MakeShareable(new FJsonValueObject(AssetInfo)));
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("assets"), AssetsArray);
        Result->SetNumberField(TEXT("asset_count"), AssetsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAnimationAssetsHandler)
