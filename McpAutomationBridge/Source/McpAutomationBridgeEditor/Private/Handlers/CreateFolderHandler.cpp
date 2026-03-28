#include "CoreMinimal.h"
#include "McpCommand.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"

class FMcpCreateFolderHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_folder"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString FolderPath;
        if (!Params->TryGetStringField(TEXT("folder_path"), FolderPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'folder_path' parameter"), TEXT("MISSING_PARAM"));
        }

        bool bCreated = UEditorAssetLibrary::MakeDirectory(FolderPath);

        if (!bCreated)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create folder: %s"), *FolderPath),
                TEXT("FOLDER_CREATION_FAILED")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("folder_path"), FolderPath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateFolderHandler)
