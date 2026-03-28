#include "CoreMinimal.h"
#include "McpCommand.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"

class FMcpMoveAssetHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("move_asset"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString SourcePath;
        if (!Params->TryGetStringField(TEXT("source_path"), SourcePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'source_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString DestPath;
        if (!Params->TryGetStringField(TEXT("dest_path"), DestPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'dest_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UObject* Asset = LoadObject<UObject>(nullptr, *SourcePath);
        if (!Asset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Asset not found: %s"), *SourcePath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        bool bMoved = UEditorAssetLibrary::RenameAsset(SourcePath, DestPath);

        if (!bMoved)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to move asset to: %s"), *DestPath),
                TEXT("MOVE_FAILED")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("moved"), true);
        Result->SetStringField(TEXT("source_path"), SourcePath);
        Result->SetStringField(TEXT("dest_path"), DestPath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpMoveAssetHandler)
