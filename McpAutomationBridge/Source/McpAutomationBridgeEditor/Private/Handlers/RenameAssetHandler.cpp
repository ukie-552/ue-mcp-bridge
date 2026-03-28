#include "CoreMinimal.h"
#include "McpCommand.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"

class FMcpRenameAssetHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("rename_asset"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AssetPath;
        if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'asset_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NewName;
        if (!Params->TryGetStringField(TEXT("new_name"), NewName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'new_name' parameter"), TEXT("MISSING_PARAM"));
        }

        UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
        if (!Asset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Asset not found: %s"), *AssetPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        FString OldName = Asset->GetName();
        bool bRenamed = UEditorAssetLibrary::RenameAsset(AssetPath, NewName);

        if (!bRenamed)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to rename asset to: %s"), *NewName),
                TEXT("RENAME_FAILED")
            );
        }

        FString NewPath = AssetPath.Replace(*OldName, *NewName);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("renamed"), true);
        Result->SetStringField(TEXT("old_name"), OldName);
        Result->SetStringField(TEXT("new_name"), NewName);
        Result->SetStringField(TEXT("old_path"), AssetPath);
        Result->SetStringField(TEXT("new_path"), NewPath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpRenameAssetHandler)
