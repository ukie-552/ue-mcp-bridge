#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "LevelEditor.h"
#include "EditorLevelLibrary.h"
#include "FileHelpers.h"
#include "AssetRegistry/AssetRegistryModule.h"

class FMcpCreateLevelHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_level"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString LevelName;
        if (!Params->TryGetStringField(TEXT("level_name"), LevelName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'level_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString PackagePath = TEXT("/Game/Maps");
        Params->TryGetStringField(TEXT("package_path"), PackagePath);

        FString TemplateName;
        Params->TryGetStringField(TEXT("template"), TemplateName);

        FString FullPath = FString::Printf(TEXT("%s/%s"), *PackagePath, *LevelName);

        UWorld* NewWorld = nullptr;

        if (!TemplateName.IsEmpty())
        {
            FString TemplatePath = FString::Printf(TEXT("%s/%s.%s"), *PackagePath, *TemplateName, *TemplateName);
            UWorld* TemplateWorld = LoadObject<UWorld>(nullptr, *TemplatePath);
            if (TemplateWorld)
            {
                NewWorld = TemplateWorld;
            }
        }

        if (!NewWorld)
        {
            NewWorld = UWorld::CreateWorld(EWorldType::Inactive, false, *LevelName);
        }

        if (!NewWorld)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create level: %s"), *LevelName),
                TEXT("LEVEL_CREATION_FAILED")
            );
        }

        FString PackageName = FString::Printf(TEXT("%s/%s"), *PackagePath, *LevelName);
        UPackage* Package = CreatePackage(*PackageName);
        
        NewWorld->Rename(*LevelName, Package, REN_None);

        FAssetRegistryModule::AssetCreated(NewWorld);
        NewWorld->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("level_name"), LevelName);
        Result->SetStringField(TEXT("level_path"), FullPath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateLevelHandler)
