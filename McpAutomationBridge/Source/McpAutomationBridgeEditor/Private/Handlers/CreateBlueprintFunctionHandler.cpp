#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"

class FMcpCreateBlueprintFunctionHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_blueprint_function"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString FunctionName;
        if (!Params->TryGetStringField(TEXT("function_name"), FunctionName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'function_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString Description;
        Params->TryGetStringField(TEXT("description"), Description);

        UBlueprint* Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(
            Blueprint,
            *FunctionName,
            UEdGraph::StaticClass(),
            UEdGraphSchema_K2::StaticClass()
        );

        if (!NewGraph)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create function: %s"), *FunctionName),
                TEXT("FUNCTION_CREATION_FAILED")
            );
        }

        Blueprint->FunctionGraphs.Add(NewGraph);
        NewGraph->Rename(nullptr, Blueprint);

        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("function_name"), FunctionName);
        Result->SetStringField(TEXT("graph_name"), NewGraph->GetName());
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateBlueprintFunctionHandler)
