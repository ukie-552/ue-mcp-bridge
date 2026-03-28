#include "CoreMinimal.h"
#include "McpCommand.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "Factories/BlueprintFactory.h"
#include "Kismet2/KismetEditorUtilities.h"

class FMcpCreateBlueprintHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_blueprint"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString BlueprintName;
        if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ParentClass = TEXT("Actor");
        Params->TryGetStringField(TEXT("parent_class"), ParentClass);

        FString FullPath = FString::Printf(TEXT("%s/%s"), *BlueprintPath, *BlueprintName);

        UClass* Parent = LoadClass<UObject>(nullptr, *ParentClass);
        if (!Parent)
        {
            Parent = AActor::StaticClass();
        }

        UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
        if (!Factory)
        {
            return FMcpCommandResult::Failure(TEXT("Failed to create blueprint factory"), TEXT("FACTORY_ERROR"));
        }

        Factory->ParentClass = Parent;

        UObject* NewAsset = Factory->FactoryCreateNew(
            UBlueprint::StaticClass(),
            GetTransientPackage(),
            *BlueprintName,
            RF_Public | RF_Standalone,
            nullptr,
            GWarn
        );

        if (!NewAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create blueprint: %s"), *BlueprintName),
                TEXT("BLUEPRINT_CREATION_FAILED")
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
        Result->SetStringField(TEXT("blueprint_name"), BlueprintName);
        Result->SetStringField(TEXT("blueprint_path"), FullPath);
        Result->SetStringField(TEXT("parent_class"), Parent->GetName());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateBlueprintHandler)
