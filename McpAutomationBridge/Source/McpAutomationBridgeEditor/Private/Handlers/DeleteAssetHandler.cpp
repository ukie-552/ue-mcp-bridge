#include "CoreMinimal.h"
#include "McpCommand.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"

class FMcpDeleteAssetHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("delete_asset"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AssetPath;
        if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'asset_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bForceDelete = false;
        Params->TryGetBoolField(TEXT("force"), bForceDelete);

        UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
        if (!Asset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Asset not found: %s"), *AssetPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<UObject*> ObjectsToDelete;
        ObjectsToDelete.Add(Asset);

        if (bForceDelete)
        {
            ObjectTools::ForceDeleteObjects(ObjectsToDelete, false);
        }
        else
        {
            ObjectTools::DeleteObjects(ObjectsToDelete, false);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("deleted"), true);
        Result->SetStringField(TEXT("asset_path"), AssetPath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpDeleteAssetHandler)
