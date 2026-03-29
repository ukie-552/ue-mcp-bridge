#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"

class FMcpGetBlueprintFunctionsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_blueprint_functions"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UBlueprint* Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> FunctionsArray;

        TArray<UEdGraph*> FunctionGraphs = Blueprint->FunctionGraphs;

        for (UEdGraph* Graph : FunctionGraphs)
        {
            if (!Graph) continue;

            TSharedPtr<FJsonObject> FuncInfo = MakeShareable(new FJsonObject);
            FuncInfo->SetStringField(TEXT("function_name"), Graph->GetFName().ToString());
            FuncInfo->SetStringField(TEXT("graph_name"), Graph->GetName());

            bool bIsUserDefined = !Graph->GetName().StartsWith(TEXT("ExecuteUbergraph"));
            FuncInfo->SetBoolField(TEXT("is_user_defined"), bIsUserDefined);

            int32 NodeCount = 0;
            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (Node)
                {
                    NodeCount++;
                }
            }
            FuncInfo->SetNumberField(TEXT("node_count"), NodeCount);

            FunctionsArray.Add(MakeShareable(new FJsonValueObject(FuncInfo)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetArrayField(TEXT("functions"), FunctionsArray);
        Result->SetNumberField(TEXT("function_count"), FunctionsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetBlueprintFunctionsHandler)
