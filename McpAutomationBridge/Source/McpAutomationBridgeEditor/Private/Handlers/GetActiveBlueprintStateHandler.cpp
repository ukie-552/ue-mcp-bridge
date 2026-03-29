#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "BlueprintEditor.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "BlueprintEditorModule.h"

class FMcpGetActiveBlueprintStateHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_active_blueprint_state"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        UBlueprint* Blueprint = nullptr;
        FString BlueprintPath;
        bool bIsActiveBlueprint = false;
        FBlueprintEditor* BlueprintEditor = nullptr;
        
        if (Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
            if (!Blueprint)
            {
                return FMcpCommandResult::Failure(
                    FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                    TEXT("ASSET_NOT_FOUND")
                );
            }
            
            IAssetEditorInstance* EditorInstance = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Blueprint, false);
            if (EditorInstance)
            {
                BlueprintEditor = static_cast<FBlueprintEditor*>(EditorInstance);
            }
        }
        else
        {
            TArray<IAssetEditorInstance*> OpenEditors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->GetAllOpenEditors();
            for (IAssetEditorInstance* Editor : OpenEditors)
            {
                if (Editor->GetEditorName() == FName("BlueprintEditor"))
                {
                    BlueprintEditor = static_cast<FBlueprintEditor*>(Editor);
                    if (BlueprintEditor)
                    {
                        Blueprint = BlueprintEditor->GetBlueprintObj();
                        if (Blueprint)
                        {
                            bIsActiveBlueprint = true;
                            break;
                        }
                    }
                }
            }
            
            if (!BlueprintEditor || !Blueprint)
            {
                return FMcpCommandResult::Failure(TEXT("No blueprint editor is currently active, please specify blueprint_path"), TEXT("NO_ACTIVE_BLUEPRINT"));
            }
        }

        UEdGraph* CurrentGraph = nullptr;
        if (BlueprintEditor)
        {
            CurrentGraph = BlueprintEditor->GetFocusedGraph();
        }
        
        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("blueprint_path"), Blueprint->GetPathName());
        Result->SetStringField(TEXT("blueprint_name"), Blueprint->GetName());
        Result->SetStringField(TEXT("current_graph"), CurrentGraph ? CurrentGraph->GetName() : TEXT(""));
        Result->SetBoolField(TEXT("is_active_blueprint"), bIsActiveBlueprint);

        TArray<UEdGraph*> GraphsToProcess;
        if (CurrentGraph)
        {
            GraphsToProcess.Add(CurrentGraph);
        }
        else
        {
            GraphsToProcess.Append(Blueprint->UbergraphPages);
            GraphsToProcess.Append(Blueprint->FunctionGraphs);
        }

        TArray<TSharedPtr<FJsonValue>> GraphsArray;
        
        for (UEdGraph* Graph : GraphsToProcess)
        {
            if (!Graph) continue;

            TSharedPtr<FJsonObject> GraphJson = MakeShareable(new FJsonObject);
            GraphJson->SetStringField(TEXT("graph_name"), Graph->GetName());
            GraphJson->SetBoolField(TEXT("is_current"), Graph == CurrentGraph);

            TArray<TSharedPtr<FJsonValue>> NodesArray;
            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (!Node) continue;

                TSharedPtr<FJsonObject> NodeJson = FMcpBlueprintUtils::NodeToJson(Node);
                if (NodeJson.IsValid())
                {
                    TArray<TSharedPtr<FJsonValue>> InputConnectionsArray;
                    TArray<TSharedPtr<FJsonValue>> OutputConnectionsArray;

                    for (UEdGraphPin* Pin : Node->Pins)
                    {
                        if (!Pin) continue;

                        for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
                        {
                            if (!LinkedPin || !LinkedPin->GetOwningNode()) continue;

                            TSharedPtr<FJsonObject> ConnectionJson = MakeShareable(new FJsonObject);
                            
                            if (Pin->Direction == EGPD_Output)
                            {
                                ConnectionJson->SetStringField(TEXT("target_node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
                                ConnectionJson->SetStringField(TEXT("source_pin_name"), Pin->PinName.ToString());
                                ConnectionJson->SetStringField(TEXT("target_pin_name"), LinkedPin->PinName.ToString());
                                OutputConnectionsArray.Add(MakeShareable(new FJsonValueObject(ConnectionJson)));
                            }
                            else
                            {
                                ConnectionJson->SetStringField(TEXT("source_node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
                                ConnectionJson->SetStringField(TEXT("source_pin_name"), LinkedPin->PinName.ToString());
                                ConnectionJson->SetStringField(TEXT("target_pin_name"), Pin->PinName.ToString());
                                InputConnectionsArray.Add(MakeShareable(new FJsonValueObject(ConnectionJson)));
                            }
                        }
                    }

                    NodeJson->SetArrayField(TEXT("input_connections"), InputConnectionsArray);
                    NodeJson->SetArrayField(TEXT("output_connections"), OutputConnectionsArray);
                    NodesArray.Add(MakeShareable(new FJsonValueObject(NodeJson)));
                }
            }

            GraphJson->SetArrayField(TEXT("nodes"), NodesArray);
            GraphsArray.Add(MakeShareable(new FJsonValueObject(GraphJson)));
        }

        Result->SetArrayField(TEXT("graphs"), GraphsArray);

        TArray<TSharedPtr<FJsonValue>> SelectedNodesArray;
        if (BlueprintEditor)
        {
            FGraphPanelSelectionSet SelectedNodes = BlueprintEditor->GetSelectedNodes();
            for (UObject* SelectedObject : SelectedNodes)
            {
                if (UEdGraphNode* SelectedNode = Cast<UEdGraphNode>(SelectedObject))
                {
                    SelectedNodesArray.Add(MakeShareable(new FJsonValueString(SelectedNode->NodeGuid.ToString())));
                }
            }
        }
        Result->SetArrayField(TEXT("selected_nodes"), SelectedNodesArray);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetActiveBlueprintStateHandler)
