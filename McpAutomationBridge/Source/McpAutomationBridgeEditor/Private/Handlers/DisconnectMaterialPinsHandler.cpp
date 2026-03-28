#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"

class FMcpDisconnectMaterialPinsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("disconnect_material_pins"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString MaterialPath;
        if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'material_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString TargetNodeName;
        if (!Params->TryGetStringField(TEXT("target_node"), TargetNodeName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'target_node' parameter"), TEXT("MISSING_PARAM"));
        }

        FString TargetInputName;
        Params->TryGetStringField(TEXT("target_input"), TargetInputName);

        UMaterial* Material = LoadObject<UMaterial>(nullptr, *MaterialPath);
        if (!Material)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Material not found: %s"), *MaterialPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UMaterialExpression* TargetExpression = FindExpression(Material, TargetNodeName);
        if (!TargetExpression)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Target node not found: %s"), *TargetNodeName),
                TEXT("TARGET_NODE_NOT_FOUND")
            );
        }

        bool bDisconnected = false;

        if (TargetInputName.IsEmpty())
        {
            for (int32 i = 0; i < 32; ++i)
            {
                FExpressionInput* Input = TargetExpression->GetInput(i);
                if (Input && Input->Expression)
                {
                    Input->Expression = nullptr;
                    Input->OutputIndex = 0;
                    bDisconnected = true;
                }
            }
        }
        else
        {
            FExpressionInput* Input = FindInput(TargetExpression, TargetInputName);
            if (Input && Input->Expression)
            {
                Input->Expression = nullptr;
                Input->OutputIndex = 0;
                bDisconnected = true;
            }
        }

        if (bDisconnected)
        {
            Material->MarkPackageDirty();
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("disconnected"), bDisconnected);
        Result->SetStringField(TEXT("target_node"), TargetNodeName);

        return FMcpCommandResult::Success(Result);
    }

private:
    UMaterialExpression* FindExpression(UMaterial* Material, const FString& NodeName)
    {
        if (!Material)
        {
            return nullptr;
        }

        TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions = Material->GetExpressions();
        for (UMaterialExpression* Expression : Expressions)
        {
            if (Expression && Expression->GetName() == NodeName)
            {
                return Expression;
            }
        }

        return nullptr;
    }

    FExpressionInput* FindInput(UMaterialExpression* Expression, const FString& InputName)
    {
        if (!Expression)
        {
            return nullptr;
        }

        for (int32 i = 0; i < 32; ++i)
        {
            FExpressionInput* Input = Expression->GetInput(i);
            if (Input)
            {
                FName InputDisplayName = Expression->GetInputName(i);
                if (InputDisplayName.ToString() == InputName || Input->InputName.ToString() == InputName)
                {
                    return Input;
                }
            }
        }

        return nullptr;
    }
};

REGISTER_MCP_COMMAND(FMcpDisconnectMaterialPinsHandler)
