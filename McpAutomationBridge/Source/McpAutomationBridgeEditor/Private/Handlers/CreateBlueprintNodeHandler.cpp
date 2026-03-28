#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "K2Node.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Variable.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "EdGraphNode_Comment.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpCreateBlueprintNodeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_blueprint_node"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NodeType;
        if (!Params->TryGetStringField(TEXT("node_type"), NodeType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'node_type' parameter"), TEXT("MISSING_PARAM"));
        }

        FString GraphName;
        Params->TryGetStringField(TEXT("graph_name"), GraphName);

        int32 PosX = 0, PosY = 0;
        const TSharedPtr<FJsonObject>* PositionObj;
        if (Params->TryGetObjectField(TEXT("position"), PositionObj))
        {
            (*PositionObj)->TryGetNumberField(TEXT("x"), PosX);
            (*PositionObj)->TryGetNumberField(TEXT("y"), PosY);
        }

        UBlueprint* Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UEdGraph* Graph = FMcpBlueprintUtils::GetGraphByName(Blueprint, GraphName);
        if (!Graph)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Graph not found: %s"), *GraphName),
                TEXT("GRAPH_NOT_FOUND")
            );
        }

        UEdGraphNode* NewNode = CreateNodeByType(Graph, NodeType, Params, PosX, PosY);
        if (!NewNode)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create node of type: %s"), *NodeType),
                TEXT("NODE_CREATION_FAILED")
            );
        }

        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> Result = FMcpBlueprintUtils::NodeToJson(NewNode);
        Result->SetStringField(TEXT("graph_name"), Graph->GetName());
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);

        return FMcpCommandResult::Success(Result);
    }

private:
    UEdGraphNode* CreateNodeByType(UEdGraph* Graph, const FString& NodeType, const TSharedPtr<FJsonObject>& Params, int32 PosX, int32 PosY)
    {
        if (!Graph)
        {
            return nullptr;
        }

        const UEdGraphSchema* Schema = Graph->GetSchema();
        if (!Schema)
        {
            return nullptr;
        }

        UEdGraphNode* NewNode = nullptr;

        if (NodeType == TEXT("Event") || NodeType == TEXT("UK2Node_Event"))
        {
            NewNode = CreateEventNode(Graph, Params);
        }
        else if (NodeType == TEXT("CallFunction") || NodeType == TEXT("UK2Node_CallFunction"))
        {
            NewNode = CreateCallFunctionNode(Graph, Params);
        }
        else if (NodeType == TEXT("VariableGet") || NodeType == TEXT("VariableSet"))
        {
            NewNode = CreateVariableNode(Graph, NodeType, Params);
        }
        else if (NodeType == TEXT("Comment"))
        {
            NewNode = CreateCommentNode(Graph, Params);
        }
        else
        {
            NewNode = CreateGenericNode(Graph, NodeType, Params);
        }

        if (NewNode)
        {
            NewNode->NodePosX = PosX;
            NewNode->NodePosY = PosY;
            Graph->NotifyGraphChanged();
        }

        return NewNode;
    }

    UEdGraphNode* CreateEventNode(UEdGraph* Graph, const TSharedPtr<FJsonObject>& Params)
    {
        FString EventName;
        Params->TryGetStringField(TEXT("event_name"), EventName);

        if (EventName.IsEmpty())
        {
            EventName = TEXT("ReceiveBeginPlay");
        }

        const UEdGraphSchema_K2* K2Schema = Cast<const UEdGraphSchema_K2>(Graph->GetSchema());
        if (!K2Schema)
        {
            return nullptr;
        }

        UClass* Class = nullptr;
        if (UBlueprint* BP = Cast<UBlueprint>(Graph->GetOuter()))
        {
            Class = BP->GeneratedClass;
        }

        UK2Node_Event* EventNode = NewObject<UK2Node_Event>(Graph);
        EventNode->SetFlags(RF_Transactional);
        Graph->AddNode(EventNode, false, false);

        UFunction* EventFunc = nullptr;
        if (Class)
        {
            EventFunc = Class->FindFunctionByName(*EventName);
        }

        if (EventFunc)
        {
            EventNode->EventReference.SetExternalMember(*EventName, EventFunc->GetOuterUClass());
        }

        EventNode->CreateNewGuid();
        EventNode->PostPlacedNewNode();
        EventNode->AllocateDefaultPins();

        return EventNode;
    }

    UEdGraphNode* CreateCallFunctionNode(UEdGraph* Graph, const TSharedPtr<FJsonObject>& Params)
    {
        FString FunctionName;
        Params->TryGetStringField(TEXT("function_name"), FunctionName);

        FString FunctionOwner;
        Params->TryGetStringField(TEXT("function_owner"), FunctionOwner);

        if (FunctionName.IsEmpty())
        {
            return nullptr;
        }

        UK2Node_CallFunction* CallNode = NewObject<UK2Node_CallFunction>(Graph);
        CallNode->SetFlags(RF_Transactional);
        Graph->AddNode(CallNode, false, false);

        UClass* OwnerClass = nullptr;
        if (!FunctionOwner.IsEmpty())
        {
            OwnerClass = LoadClass<UObject>(nullptr, *FunctionOwner);
        }

        if (OwnerClass)
        {
            UFunction* Func = OwnerClass->FindFunctionByName(*FunctionName);
            if (Func)
            {
                CallNode->SetFromFunction(Func);
            }
        }
        else
        {
            CallNode->FunctionReference.SetMemberName(*FunctionName);
        }

        CallNode->CreateNewGuid();
        CallNode->PostPlacedNewNode();
        CallNode->AllocateDefaultPins();

        return CallNode;
    }

    UEdGraphNode* CreateVariableNode(UEdGraph* Graph, const FString& NodeType, const TSharedPtr<FJsonObject>& Params)
    {
        FString VariableName;
        Params->TryGetStringField(TEXT("variable_name"), VariableName);

        if (VariableName.IsEmpty())
        {
            return nullptr;
        }

        UK2Node_Variable* VarNode = nullptr;

        if (NodeType == TEXT("VariableGet"))
        {
            UK2Node_VariableGet* GetNode = NewObject<UK2Node_VariableGet>(Graph);
            GetNode->SetFlags(RF_Transactional);
            Graph->AddNode(GetNode, false, false);
            GetNode->VariableReference.SetMemberName(*VariableName);
            VarNode = GetNode;
        }
        else
        {
            UK2Node_VariableSet* SetNode = NewObject<UK2Node_VariableSet>(Graph);
            SetNode->SetFlags(RF_Transactional);
            Graph->AddNode(SetNode, false, false);
            SetNode->VariableReference.SetMemberName(*VariableName);
            VarNode = SetNode;
        }

        if (VarNode)
        {
            VarNode->CreateNewGuid();
            VarNode->PostPlacedNewNode();
            VarNode->AllocateDefaultPins();
        }

        return VarNode;
    }

    UEdGraphNode* CreateCommentNode(UEdGraph* Graph, const TSharedPtr<FJsonObject>& Params)
    {
        FString CommentText;
        Params->TryGetStringField(TEXT("comment_text"), CommentText);

        UEdGraphNode_Comment* CommentNode = NewObject<UEdGraphNode_Comment>(Graph);
        CommentNode->SetFlags(RF_Transactional);
        Graph->AddNode(CommentNode, false, false);

        if (!CommentText.IsEmpty())
        {
            CommentNode->NodeComment = CommentText;
        }

        CommentNode->CreateNewGuid();
        CommentNode->PostPlacedNewNode();

        return CommentNode;
    }

    UEdGraphNode* CreateGenericNode(UEdGraph* Graph, const FString& NodeType, const TSharedPtr<FJsonObject>& Params)
    {
        UClass* NodeClass = LoadClass<UEdGraphNode>(nullptr, *NodeType);
        if (!NodeClass)
        {
            FString FullClassName = FString::Printf(TEXT("Engine.%s"), *NodeType);
            NodeClass = LoadClass<UEdGraphNode>(nullptr, *FullClassName);
        }

        if (!NodeClass)
        {
            return nullptr;
        }

        UEdGraphNode* NewNode = NewObject<UEdGraphNode>(Graph, NodeClass);
        NewNode->SetFlags(RF_Transactional);
        Graph->AddNode(NewNode, false, false);

        NewNode->CreateNewGuid();
        NewNode->PostPlacedNewNode();
        NewNode->AllocateDefaultPins();

        return NewNode;
    }
};

REGISTER_MCP_COMMAND(FMcpCreateBlueprintNodeHandler)
