#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"

class FMcpGetAnimBlueprintGraphsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_anim_blueprint_graphs"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *BlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> GraphsArray;

        for (UEdGraph* Graph : AnimBP->UbergraphPages)
        {
            if (!Graph) continue;

            TSharedPtr<FJsonObject> GraphInfo = MakeShareable(new FJsonObject);
            GraphInfo->SetStringField(TEXT("graph_name"), Graph->GetName());
            GraphInfo->SetStringField(TEXT("graph_type"), TEXT("Ubergraph"));

            int32 NodeCount = 0;
            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (Node) NodeCount++;
            }
            GraphInfo->SetNumberField(TEXT("node_count"), NodeCount);

            GraphsArray.Add(MakeShareable(new FJsonValueObject(GraphInfo)));
        }

        for (UEdGraph* Graph : AnimBP->FunctionGraphs)
        {
            if (!Graph) continue;

            TSharedPtr<FJsonObject> GraphInfo = MakeShareable(new FJsonObject);
            GraphInfo->SetStringField(TEXT("graph_name"), Graph->GetName());
            GraphInfo->SetStringField(TEXT("graph_type"), TEXT("Function"));

            int32 NodeCount = 0;
            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (Node) NodeCount++;
            }
            GraphInfo->SetNumberField(TEXT("node_count"), NodeCount);

            GraphsArray.Add(MakeShareable(new FJsonValueObject(GraphInfo)));
        }

        for (UEdGraph* Graph : AnimBP->MacroGraphs)
        {
            if (!Graph) continue;

            TSharedPtr<FJsonObject> GraphInfo = MakeShareable(new FJsonObject);
            GraphInfo->SetStringField(TEXT("graph_name"), Graph->GetName());
            GraphInfo->SetStringField(TEXT("graph_type"), TEXT("Macro"));

            int32 NodeCount = 0;
            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (Node) NodeCount++;
            }
            GraphInfo->SetNumberField(TEXT("node_count"), NodeCount);

            GraphsArray.Add(MakeShareable(new FJsonValueObject(GraphInfo)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetArrayField(TEXT("graphs"), GraphsArray);
        Result->SetNumberField(TEXT("graph_count"), GraphsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAnimBlueprintGraphsHandler)
