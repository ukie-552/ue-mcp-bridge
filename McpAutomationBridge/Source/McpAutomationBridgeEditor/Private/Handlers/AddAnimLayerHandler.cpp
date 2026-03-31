#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimClassInterface.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpAddAnimLayerHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("add_anim_layer"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString LayerName;
        if (!Params->TryGetStringField(TEXT("layer_name"), LayerName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'layer_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString InterfacePath;
        if (!Params->TryGetStringField(TEXT("interface_path"), InterfacePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'interface_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UClass* InterfaceClass = LoadClass<UObject>(nullptr, *InterfacePath);
        if (!InterfaceClass)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Interface not found: %s"), *InterfacePath),
                TEXT("INTERFACE_NOT_FOUND")
            );
        }

        AnimBP->Modify();

        FName LayerFName = FName(*LayerName);

        AnimBP->LayerInterfaces.Add(LayerFName);

        FAnimBlueprintFunction NewFunction;
        NewFunction.Name = LayerFName;
        NewFunction.InputProperties = TArray<FProperty*>();
        NewFunction.OutputProperty = nullptr;
        AnimBP->Functions.Add(NewFunction);

        UEdGraph* LayerGraph = FBlueprintEditorUtils::AddFunctionGraph(AnimBP, LayerFName, FEdGraphPinType(), nullptr);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("added"), true);
        Result->SetStringField(TEXT("layer_name"), LayerName);
        Result->SetStringField(TEXT("interface_path"), InterfacePath);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
        Result->SetStringField(TEXT("graph_name"), LayerGraph ? LayerGraph->GetName() : TEXT(""));

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpAddAnimLayerHandler)