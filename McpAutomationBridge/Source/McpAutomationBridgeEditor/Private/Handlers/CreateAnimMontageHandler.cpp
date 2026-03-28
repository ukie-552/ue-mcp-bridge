#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

class FMcpCreateAnimMontageHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_anim_montage"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString PackagePath;
        if (!Params->TryGetStringField(TEXT("package_path"), PackagePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'package_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString MontageName;
        if (!Params->TryGetStringField(TEXT("montage_name"), MontageName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'montage_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString AnimSequencePath;
        if (!Params->TryGetStringField(TEXT("anim_sequence"), AnimSequencePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_sequence' parameter"), TEXT("MISSING_PARAM"));
        }

        UAnimSequence* AnimSequence = LoadObject<UAnimSequence>(nullptr, *AnimSequencePath);
        if (!AnimSequence)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("AnimSequence not found: %s"), *AnimSequencePath),
                TEXT("ANIM_SEQUENCE_NOT_FOUND")
            );
        }

        FString FullPath = FString::Printf(TEXT("%s/%s"), *PackagePath, *MontageName);

        FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
        
        UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
            MontageName,
            PackagePath,
            UAnimMontage::StaticClass(),
            nullptr
        );

        if (!NewAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create anim montage: %s"), *MontageName),
                TEXT("MONTAGE_CREATION_FAILED")
            );
        }

        UAnimMontage* NewMontage = Cast<UAnimMontage>(NewAsset);
        if (NewMontage)
        {
            FAssetRegistryModule::AssetCreated(NewMontage);
            NewMontage->MarkPackageDirty();
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("montage_name"), MontageName);
        Result->SetStringField(TEXT("montage_path"), FullPath);
        Result->SetStringField(TEXT("source_sequence"), AnimSequencePath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateAnimMontageHandler)
