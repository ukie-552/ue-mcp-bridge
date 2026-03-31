#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"

class FMcpGetDetailsPanelCategoriesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_details_panel_categories"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ObjectType;
        if (!Params->TryGetStringField(TEXT("object_type"), ObjectType))
        {
            ObjectType = TEXT("actor");
        }

        FString ObjectId;
        if (!Params->TryGetStringField(TEXT("object_id"), ObjectId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'object_id' parameter"), TEXT("MISSING_PARAM"));
        }

        UObject* TargetObject = LoadObject<UObject>(nullptr, *ObjectId);
        if (!TargetObject)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Object not found: %s"), *ObjectId),
                TEXT("OBJECT_NOT_FOUND")
            );
        }

        UClass* ClassToInspect = TargetObject->GetClass();
        TSet<FString> Categories;

        for (TFieldIterator<FProperty> PropIt(ClassToInspect); PropIt; ++PropIt)
        {
            FProperty* Property = *PropIt;
            
            if (!Property->HasAnyPropertyFlags(CPF_Edit))
            {
                continue;
            }

            FString Category = Property->GetMetaData(TEXT("Category"));
            if (Category.IsEmpty())
            {
                Category = TEXT("Default");
            }

            Categories.Add(Category);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        
        TArray<TSharedPtr<FJsonValue>> CategoriesArray;
        for (const FString& Category : Categories)
        {
            CategoriesArray.Add(MakeShareable(new FJsonValueString(Category)));
        }
        
        CategoriesArray.Sort([](const TSharedPtr<FJsonValue>& A, const TSharedPtr<FJsonValue>& B) {
            return A->AsString() < B->AsString();
        });

        Result->SetStringField(TEXT("object_type"), ObjectType);
        Result->SetStringField(TEXT("object_id"), ObjectId);
        Result->SetArrayField(TEXT("categories"), CategoriesArray);
        Result->SetNumberField(TEXT("category_count"), Categories.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetDetailsPanelCategoriesHandler)