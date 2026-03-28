#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"

class FMcpGetBlueprintNodesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_blueprint_nodes"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString GraphName;
        Params->TryGetStringField(TEXT("graph_name"), GraphName);

        UBlueprint* Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<UEdGraph*> GraphsToProcess;
        
        if (!GraphName.IsEmpty())
        {
            UEdGraph* TargetGraph = FMcpBlueprintUtils::GetGraphByName(Blueprint, GraphName);
            if (TargetGraph)
            {
                GraphsToProcess.Add(TargetGraph);
            }
        }
        else
        {
            GraphsToProcess.Append(Blueprint->UbergraphPages);
            GraphsToProcess.Append(Blueprint->FunctionGraphs);
        }

        TArray<TSharedPtr<FJsonValue>> NodesArray;
        for (UEdGraph* Graph : GraphsToProcess)
        {
            if (!Graph) continue;

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (!Node) continue;

                TSharedPtr<FJsonObject> NodeJson = FMcpBlueprintUtils::NodeToJson(Node);
                if (NodeJson.IsValid())
                {
                    NodeJson->SetStringField(TEXT("graph_name"), Graph->GetName());
                    NodeJson->SetStringField(TEXT("blueprint_path"), BlueprintPath);
                    NodesArray.Add(MakeShareable(new FJsonValueObject(NodeJson)));
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("nodes"), NodesArray);
        Result->SetNumberField(TEXT("count"), NodesArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetBlueprintNodesHandler)
