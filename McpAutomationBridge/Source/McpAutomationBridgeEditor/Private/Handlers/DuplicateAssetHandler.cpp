#include "CoreMinimal.h"
#include "McpCommand.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"

class FMcpDuplicateAssetHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("duplicate_asset"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SourceAssetPath;
        if (!Params->TryGetStringField(TEXT("source_asset_path"), SourceAssetPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'source_asset_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString DestAssetPath;
        if (!Params->TryGetStringField(TEXT("dest_asset_path"), DestAssetPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'dest_asset_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UObject* SourceAsset = LoadObject<UObject>(nullptr, *SourceAssetPath);
        if (!SourceAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Source asset not found: %s"), *SourceAssetPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UObject* DuplicatedAsset = UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, DestAssetPath);
        if (!DuplicatedAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to duplicate asset to: %s"), *DestAssetPath),
                TEXT("DUPLICATE_FAILED")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("duplicated"), true);
        Result->SetStringField(TEXT("source_asset_path"), SourceAssetPath);
        Result->SetStringField(TEXT("dest_asset_path"), DestAssetPath);
        Result->SetStringField(TEXT("asset_name"), DuplicatedAsset->GetName());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpDuplicateAssetHandler)
