#include "CoreMinimal.h"
#include "McpCommand.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "WidgetBlueprint.h"
#include "Blueprint/UserWidget.h"
#include "Kismet2/KismetEditorUtilities.h"

class FMcpCreateWidgetBlueprintHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_widget_blueprint"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString WidgetPath;
        if (!Params->TryGetStringField(TEXT("widget_path"), WidgetPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'widget_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString WidgetName;
        if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'widget_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ParentClassPath;
        Params->TryGetStringField(TEXT("parent_class"), ParentClassPath);

        FString FullPath = FString::Printf(TEXT("%s/%s"), *WidgetPath, *WidgetName);

        FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

        UClass* ParentClass = UUserWidget::StaticClass();
        if (!ParentClassPath.IsEmpty())
        {
            UClass* LoadedClass = LoadClass<UUserWidget>(nullptr, *ParentClassPath);
            if (LoadedClass)
            {
                ParentClass = LoadedClass;
            }
        }

        UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
            WidgetName,
            WidgetPath,
            UWidgetBlueprint::StaticClass(),
            nullptr
        );

        if (!NewAsset)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create widget blueprint: %s"), *WidgetName),
                TEXT("WIDGET_CREATION_FAILED")
            );
        }

        UWidgetBlueprint* NewWidgetBP = Cast<UWidgetBlueprint>(NewAsset);
        if (NewWidgetBP)
        {
            FAssetRegistryModule::AssetCreated(NewWidgetBP);
            NewWidgetBP->MarkPackageDirty();
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("widget_name"), WidgetName);
        Result->SetStringField(TEXT("widget_path"), FullPath);
        Result->SetStringField(TEXT("parent_class"), ParentClass->GetPathName());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateWidgetBlueprintHandler)
