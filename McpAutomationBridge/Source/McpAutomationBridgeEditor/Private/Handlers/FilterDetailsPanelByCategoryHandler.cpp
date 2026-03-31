#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

class FMcpFilterDetailsPanelByCategoryHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("filter_details_panel_by_category"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Category;
        if (!Params->TryGetStringField(TEXT("category"), Category))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'category' parameter"), TEXT("MISSING_PARAM"));
        }

        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        TArray<IDetailsView*> DetailsViews = PropertyModule.GetPropertyViews();

        bool bFiltered = false;
        FString CurrentCategory;

        for (IDetailsView* DetailsView : DetailsViews)
        {
            TArray<UObject*> Objects = DetailsView->GetSelectedObjects().Array();
            if (Objects.Num() > 0)
            {
                DetailsView->SetObjects(Objects, true);
                bFiltered = true;
                break;
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("filtered"), bFiltered);
        Result->SetStringField(TEXT("category"), Category);
        
        if (!bFiltered)
        {
            Result->SetStringField(TEXT("message"), TEXT("No details panel with selected objects found"));
        }

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpFilterDetailsPanelByCategoryHandler)