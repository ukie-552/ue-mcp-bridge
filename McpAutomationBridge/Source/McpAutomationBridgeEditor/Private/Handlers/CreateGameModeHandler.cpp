#include "CoreMinimal.h"
#include "McpCommand.h"
#include "GameFramework/GameModeBase.h"
#include "Factories/BlueprintFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/KismetEditorUtilities.h"

class FMcpCreateGameModeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_game_mode"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString PackagePath;
        if (!Params->TryGetStringField(TEXT("package_path"), PackagePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'package_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString GameModeName;
        if (!Params->TryGetStringField(TEXT("game_mode_name"), GameModeName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'game_mode_name' parameter"), TEXT("MISSING_PARAM"));
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
            ParentClass = AGameModeBase::StaticClass();
        }

        UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
        if (!Factory)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to create blueprint factory"), TEXT("FACTORY_ERROR"));
        }

        Factory->ParentClass = ParentClass;

        FString FullPath = FString::Printf(TEXT("%s/%s"), *PackagePath, *GameModeName);

        UObject* NewAsset = Factory->FactoryCreateNew(
            UBlueprint::StaticClass(),
            GetTransientPackage(),
            *GameModeName,
            RF_Public | RF_Standalone,
            nullptr,
            GWarn
        );

        if (!NewAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create game mode blueprint: %s"), *GameModeName),
                TEXT("GAME_MODE_CREATION_FAILED")
            );
        }

        UBlueprint* NewBlueprint = Cast<UBlueprint>(NewAsset);
        if (NewBlueprint)
        {
            FAssetRegistryModule::AssetCreated(NewBlueprint);
            NewBlueprint->MarkPackageDirty();
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("game_mode_name"), GameModeName);
        Result->SetStringField(TEXT("game_mode_path"), FullPath);
        Result->SetStringField(TEXT("parent_class"), ParentClass->GetName());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateGameModeHandler)
