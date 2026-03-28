#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/AnimBlueprintFactory.h"

class FMcpCreateAnimBlueprintHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_anim_blueprint"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString PackagePath;
        if (!Params->TryGetStringField(TEXT("package_path"), PackagePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'package_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString AnimBlueprintName;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_name"), AnimBlueprintName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString TargetSkeletonPath;
        if (!Params->TryGetStringField(TEXT("target_skeleton"), TargetSkeletonPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'target_skeleton' parameter"), TEXT("MISSING_PARAM"));
        }

        USkeleton* TargetSkeleton = LoadObject<USkeleton>(nullptr, *TargetSkeletonPath);
        if (!TargetSkeleton)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Skeleton not found: %s"), *TargetSkeletonPath),
                TEXT("SKELETON_NOT_FOUND")
            );
        }

        FString ParentClassPath;
        Params->TryGetStringField(TEXT("parent_class"), ParentClassPath);

        UClass* ParentClass = nullptr;
        if (!ParentClassPath.IsEmpty())
        {
            ParentClass = LoadClass<UObject>(nullptr, *ParentClassPath);
        }

        if (!ParentClass)
        {
            ParentClass = UAnimInstance::StaticClass();
        }

        UAnimBlueprintFactory* Factory = NewObject<UAnimBlueprintFactory>();
        if (!Factory)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to create anim blueprint factory"), TEXT("FACTORY_ERROR"));
        }

        Factory->TargetSkeleton = TargetSkeleton;
        Factory->ParentClass = ParentClass;

        FString FullPath = FString::Printf(TEXT("%s/%s"), *PackagePath, *AnimBlueprintName);

        UPackage* TargetPackage = CreatePackage(*FullPath);
        if (!TargetPackage)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create package: %s"), *FullPath),
                TEXT("PACKAGE_ERROR")
            );
        }

        UObject* NewAsset = Factory->FactoryCreateNew(
            UAnimBlueprint::StaticClass(),
            TargetPackage,
            *AnimBlueprintName,
            RF_Public | RF_Standalone,
            nullptr,
            GWarn
        );

        if (!NewAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create anim blueprint: %s"), *AnimBlueprintName),
                TEXT("ANIM_BP_CREATION_FAILED")
            );
        }

        UAnimBlueprint* NewAnimBP = Cast<UAnimBlueprint>(NewAsset);
        if (NewAnimBP)
        {
        }

        FAssetRegistryModule::AssetCreated(NewAsset);
        NewAsset->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("anim_blueprint_name"), AnimBlueprintName);
        Result->SetStringField(TEXT("anim_blueprint_path"), FullPath);
        Result->SetStringField(TEXT("target_skeleton"), TargetSkeletonPath);
        Result->SetStringField(TEXT("parent_class"), ParentClass->GetName());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateAnimBlueprintHandler)
