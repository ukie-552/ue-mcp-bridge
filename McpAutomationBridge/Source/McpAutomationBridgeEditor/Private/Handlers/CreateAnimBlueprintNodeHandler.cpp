#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AnimNode_BlendSpacePlayer.h"
#include "AnimGraphNode_SequencePlayer.h"
#include "AnimGraphNode_BlendSpacePlayer.h"
#include "AnimGraphNode_StateMachine.h"
#include "AnimGraphNode_LinkedAnimLayer.h"
#include "AnimGraphNode_LinkedAnimGraph.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

class FMcpCreateAnimBlueprintNodeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_anim_node"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NodeType;
        if (!Params->TryGetStringField(TEXT("node_type"), NodeType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'node_type' parameter"), TEXT("MISSING_PARAM"));
        }

        FString GraphName = TEXT("AnimGraph");
        Params->TryGetStringField(TEXT("graph_name"), GraphName);

        float PosX = 0.0f;
        float PosY = 0.0f;
        if (Params->HasField(TEXT("position")))
        {
            TSharedPtr<FJsonObject> PositionObj = Params->GetObjectField(TEXT("position"));
            PosX = PositionObj->GetNumberField(TEXT("x"));
            PosY = PositionObj->GetNumberField(TEXT("y"));
        }

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UEdGraph* TargetGraph = nullptr;
        for (UEdGraph* Graph : AnimBP->UbergraphPages)
        {
            if (Graph && Graph->GetName() == GraphName)
            {
                TargetGraph = Graph;
                break;
            }
        }

        if (!TargetGraph)
        {
            for (UEdGraph* Graph : AnimBP->UbergraphPages)
            {
                if (Graph && Graph->IsA<UAnimationGraph>())
                {
                    TargetGraph = Graph;
                    break;
                }
            }
        }

        if (!TargetGraph)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Graph not found: %s"), *GraphName),
                TEXT("GRAPH_NOT_FOUND")
            );
        }

        UEdGraphNode* NewNode = CreateAnimNode(AnimBP, TargetGraph, NodeType, PosX, PosY);

        if (!NewNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create node of type: %s"), *NodeType),
                TEXT("NODE_CREATION_FAILED")
            );
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("node_type"), NodeType);
        Result->SetStringField(TEXT("node_id"), NewNode->NodeGuid.ToString());
        Result->SetStringField(TEXT("node_name"), NewNode->GetName());
        Result->SetNumberField(TEXT("pos_x"), NewNode->NodePosX);
        Result->SetNumberField(TEXT("pos_y"), NewNode->NodePosY);

        return FMcpCommandResult::Success(Result);
    }

private:
    UEdGraphNode* CreateAnimNode(UAnimBlueprint* AnimBP, UEdGraph* Graph, const FString& NodeType, float PosX, float PosY)
    {
        UEdGraphNode* NewNode = nullptr;

        if (NodeType == TEXT("SequencePlayer"))
        {
            UAnimGraphNode_SequencePlayer* PlayerNode = NewObject<UAnimGraphNode_SequencePlayer>(Graph);
            if (PlayerNode)
            {
                PlayerNode->NodePosX = PosX;
                PlayerNode->NodePosY = PosY;
                PlayerNode->CreateNewGuid();
                PlayerNode->PostPlacedNewNode();
                PlayerNode->AllocateDefaultPins();
                Graph->Modify();
                Graph->AddNode(PlayerNode);
                NewNode = PlayerNode;
            }
        }
        else if (NodeType == TEXT("BlendSpacePlayer"))
        {
            UAnimGraphNode_BlendSpacePlayer* BlendSpaceNode = NewObject<UAnimGraphNode_BlendSpacePlayer>(Graph);
            if (BlendSpaceNode)
            {
                BlendSpaceNode->NodePosX = PosX;
                BlendSpaceNode->NodePosY = PosY;
                BlendSpaceNode->CreateNewGuid();
                BlendSpaceNode->PostPlacedNewNode();
                BlendSpaceNode->AllocateDefaultPins();
                Graph->Modify();
                Graph->AddNode(BlendSpaceNode);
                NewNode = BlendSpaceNode;
            }
        }
        else if (NodeType == TEXT("StateMachine"))
        {
            UAnimGraphNode_StateMachine* SMNode = NewObject<UAnimGraphNode_StateMachine>(Graph);
            if (SMNode)
            {
                SMNode->NodePosX = PosX;
                SMNode->NodePosY = PosY;
                SMNode->CreateNewGuid();
                SMNode->PostPlacedNewNode();
                SMNode->AllocateDefaultPins();
                Graph->Modify();
                Graph->AddNode(SMNode);
                NewNode = SMNode;
            }
        }
        else if (NodeType == TEXT("LinkedAnimLayer"))
        {
            UAnimGraphNode_LinkedAnimLayer* LayerNode = NewObject<UAnimGraphNode_LinkedAnimLayer>(Graph);
            if (LayerNode)
            {
                LayerNode->NodePosX = PosX;
                LayerNode->NodePosY = PosY;
                LayerNode->CreateNewGuid();
                LayerNode->PostPlacedNewNode();
                LayerNode->AllocateDefaultPins();
                Graph->Modify();
                Graph->AddNode(LayerNode);
                NewNode = LayerNode;
            }
        }
        else if (NodeType == TEXT("LinkedAnimGraph"))
        {
            UAnimGraphNode_LinkedAnimGraph* GraphNode = NewObject<UAnimGraphNode_LinkedAnimGraph>(Graph);
            if (GraphNode)
            {
                GraphNode->NodePosX = PosX;
                GraphNode->NodePosY = PosY;
                GraphNode->CreateNewGuid();
                GraphNode->PostPlacedNewNode();
                GraphNode->AllocateDefaultPins();
                Graph->Modify();
                Graph->AddNode(GraphNode);
                NewNode = GraphNode;
            }
        }

        return NewNode;
    }
};

REGISTER_MCP_COMMAND(FMcpCreateAnimBlueprintNodeHandler)