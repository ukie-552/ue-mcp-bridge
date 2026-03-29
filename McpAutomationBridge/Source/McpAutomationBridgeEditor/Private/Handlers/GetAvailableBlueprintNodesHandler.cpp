#include "CoreMinimal.h"
#include "McpCommand.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraph.h"
#include "K2Node.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabase.h"

class FMcpGetAvailableBlueprintNodesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_available_blueprint_nodes"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString CategoryFilter;
        Params->TryGetStringField(TEXT("category"), CategoryFilter);

        FString SearchTerm;
        Params->TryGetStringField(TEXT("search"), SearchTerm);

        int32 MaxResults = 500;
        Params->TryGetNumberField(TEXT("max_results"), MaxResults);

        bool bIncludeKeywords = false;
        Params->TryGetBoolField(TEXT("include_keywords"), bIncludeKeywords);

        FBlueprintActionDatabase& Database = FBlueprintActionDatabase::Get();
        const FBlueprintActionDatabase::FActionRegistry& AllActions = Database.GetAllActions();

        TArray<TSharedPtr<FJsonValue>> NodesArray;
        TSet<FString> UniqueNodeNames;
        TSet<FString> Categories;

        for (const auto& Pair : AllActions)
        {
            for (const TObjectPtr<UBlueprintNodeSpawner>& Spawner : Pair.Value)
            {
                if (!Spawner.IsValid())
                {
                    continue;
                }

                FBlueprintActionUiSpec UiSpec = Spawner->PrimeDefaultUiSpec();

                FString NodeName = UiSpec.MenuName.ToString();
                FString NodeCategory = UiSpec.Category.ToString();
                FString NodeTooltip = UiSpec.Tooltip.ToString();
                FString NodeKeywords = UiSpec.Keywords.ToString();
                FString NodeClass = Spawner->NodeClass ? Spawner->NodeClass->GetName() : TEXT("");

                if (NodeName.IsEmpty())
                {
                    continue;
                }

                if (!CategoryFilter.IsEmpty() && !NodeCategory.Contains(CategoryFilter))
                {
                    continue;
                }

                if (!SearchTerm.IsEmpty())
                {
                    bool bMatches = NodeName.Contains(SearchTerm) || 
                                   NodeTooltip.Contains(SearchTerm) ||
                                   NodeKeywords.Contains(SearchTerm) ||
                                   NodeClass.Contains(SearchTerm);
                    if (!bMatches)
                    {
                        continue;
                    }
                }

                FString UniqueKey = FString::Printf(TEXT("%s|%s"), *NodeCategory, *NodeName);
                if (UniqueNodeNames.Contains(UniqueKey))
                {
                    continue;
                }
                UniqueNodeNames.Add(UniqueKey);

                TSharedPtr<FJsonObject> NodeInfo = MakeShareable(new FJsonObject);
                NodeInfo->SetStringField(TEXT("name"), NodeName);
                NodeInfo->SetStringField(TEXT("category"), NodeCategory);
                NodeInfo->SetStringField(TEXT("tooltip"), NodeTooltip);
                NodeInfo->SetStringField(TEXT("node_class"), NodeClass);

                if (bIncludeKeywords && !NodeKeywords.IsEmpty())
                {
                    NodeInfo->SetStringField(TEXT("keywords"), NodeKeywords);
                }

                NodesArray.Add(MakeShareable(new FJsonValueObject(NodeInfo)));
                Categories.Add(NodeCategory);

                if (NodesArray.Num() >= MaxResults)
                {
                    break;
                }
            }

            if (NodesArray.Num() >= MaxResults)
            {
                break;
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("nodes"), NodesArray);
        Result->SetNumberField(TEXT("count"), NodesArray.Num());
        Result->SetNumberField(TEXT("total_available"), UniqueNodeNames.Num());

        TArray<TSharedPtr<FJsonValue>> CategoriesArray;
        for (const FString& Cat : Categories)
        {
            CategoriesArray.Add(MakeShareable(new FJsonValueString(Cat)));
        }
        Result->SetArrayField(TEXT("categories"), CategoriesArray);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAvailableBlueprintNodesHandler)
