#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"

class FMcpCreateAnimBlueprintFunctionHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_anim_blueprint_function"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString FunctionName;
        if (!Params->TryGetStringField(TEXT("function_name"), FunctionName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'function_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString Description;
        Params->TryGetStringField(TEXT("description"), Description);

        bool bIsMacro = false;
        Params->TryGetBoolField(TEXT("is_macro"), bIsMacro);

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(
            AnimBP,
            *FunctionName,
            bIsMacro ? UEdGraph_Macro::StaticClass() : UEdGraph::StaticClass(),
            UEdGraphSchema_K2::StaticClass()
        );

        if (!NewGraph)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create function: %s"), *FunctionName),
                TEXT("FUNCTION_CREATION_FAILED")
            );
        }

        if (bIsMacro)
        {
            AnimBP->MacroGraphs.Add(NewGraph);
        }
        else
        {
            AnimBP->FunctionGraphs.Add(NewGraph);
        }

        NewGraph->Rename(nullptr, AnimBP);

        FBlueprintEditorUtils::MarkBlueprintAsModified(AnimBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("function_name"), FunctionName);
        Result->SetStringField(TEXT("graph_name"), NewGraph->GetName());
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetBoolField(TEXT("is_macro"), bIsMacro);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateAnimBlueprintFunctionHandler)