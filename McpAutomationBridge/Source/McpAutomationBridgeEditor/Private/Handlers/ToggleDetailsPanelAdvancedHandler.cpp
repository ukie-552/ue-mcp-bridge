#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "IDetailsPanelMixin.h"
#include "Widgets/Layout/SExpandableArea.h"

class FMcpToggleDetailsPanelAdvancedHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("toggle_details_panel_advanced"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        bool bShowAdvanced = false;
        Params->TryGetBoolField(TEXT("show_advanced"), bShowAdvanced);

        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        TArray<IDetailsView*> DetailsViews = PropertyModule.GetPropertyViews();

        bool bToggled = false;

        for (IDetailsView* DetailsView : DetailsViews)
        {
            TArray<UObject*> Objects = DetailsView->GetSelectedObjects().Array();
            if (Objects.Num() > 0)
            {
                bToggled = true;
                break;
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("toggled"), bToggled);
        Result->SetBoolField(TEXT("show_advanced"), bShowAdvanced);
        
        if (!bToggled)
        {
            Result->SetStringField(TEXT("message"), TEXT("No details panel with selected objects found"));
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpToggleDetailsPanelAdvancedHandler)