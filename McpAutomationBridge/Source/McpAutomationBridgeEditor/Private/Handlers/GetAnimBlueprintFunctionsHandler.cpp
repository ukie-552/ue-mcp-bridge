#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"

class FMcpGetAnimBlueprintFunctionsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_anim_blueprint_functions"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> FunctionsArray;

        TArray<UEdGraph*> FunctionGraphs;
        FunctionGraphs.Append(AnimBP->FunctionGraphs);
        FunctionGraphs.Append(AnimBP->MacroGraphs);

        for (UEdGraph* Graph : FunctionGraphs)
        {
            if (!Graph) continue;

            TSharedPtr<FJsonObject> FuncInfo = MakeShareable(new FJsonObject);
            FuncInfo->SetStringField(TEXT("function_name"), Graph->GetFName().ToString());
            FuncInfo->SetStringField(TEXT("graph_name"), Graph->GetName());

            bool bIsUserDefined = !Graph->GetName().StartsWith(TEXT("ExecuteUbergraph"));
            FuncInfo->SetBoolField(TEXT("is_user_defined"), bIsUserDefined);

            bool bIsMacro = Graph->IsA<UEdGraph_Macro>();
            FuncInfo->SetBoolField(TEXT("is_macro"), bIsMacro);

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
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetArrayField(TEXT("functions"), FunctionsArray);
        Result->SetNumberField(TEXT("function_count"), FunctionsArray.Num());

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpGetAnimBlueprintFunctionsHandler)