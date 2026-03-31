#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "PropertyHandle.h"
#include "IPropertyUtilities.h"

class FMcpResetDetailsPanelPropertyHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("reset_details_panel_property"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ObjectPath;
        if (!Params->TryGetStringField(TEXT("object_path"), ObjectPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'object_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString PropertyPath;
        if (!Params->TryGetStringField(TEXT("property_path"), PropertyPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'property_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UObject* TargetObject = LoadObject<UObject>(nullptr, *ObjectPath);
        if (!TargetObject)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Object not found: %s"), *ObjectPath),
                TEXT("OBJECT_NOT_FOUND")
            );
        }

        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        TArray<IDetailsView*> DetailsViews = PropertyModule.GetPropertyViews();

        bool bReset = false;
        FString ResetPropertyName;

        for (IDetailsView* DetailsView : DetailsViews)
        {
            const TArray<TWeakObjectPtr<UObject>>& ViewedObjects = DetailsView->GetSelectedObjects();
            
            for (const TWeakObjectPtr<UObject>& Obj : ViewedObjects)
            {
                if (Obj.IsValid() && Obj.Get() == TargetObject)
                {
                    TSharedPtr<FPropertyHandle> PropertyHandle = DetailsView->GetPropertyHandle(PropertyPath);
                    if (PropertyHandle.IsValid())
                    {
                        PropertyHandle->ResetToDefault();
                        PropertyHandle->NotifyFinishedChangingProperties();
                        bReset = true;
                        ResetPropertyName = PropertyPath;
                        break;
                    }
                }
            }
            
            if (bReset) break;
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("reset"), bReset);
        Result->SetStringField(TEXT("property_path"), ResetPropertyName);
        Result->SetStringField(TEXT("object_path"), ObjectPath);

        if (!bReset)
        {
            Result->SetStringField(TEXT("message"), TEXT("Property handle not found in current details view"));
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpResetDetailsPanelPropertyHandler)