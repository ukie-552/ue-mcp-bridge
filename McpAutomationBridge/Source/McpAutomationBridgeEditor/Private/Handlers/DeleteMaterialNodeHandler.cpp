#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"

class FMcpDeleteMaterialNodeHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("delete_material_node"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString MaterialPath;
        if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'material_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString NodeName;
        if (!Params->TryGetStringField(TEXT("node_name"), NodeName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'node_name' parameter"), TEXT("MISSING_PARAM"));
        }

        UMaterial* Material = LoadObject<UMaterial>(nullptr, *MaterialPath);
        if (!Material)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Material not found: %s"), *MaterialPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        UMaterialExpression* Expression = FindExpression(Material, NodeName);
        if (!Expression)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Node not found: %s"), *NodeName),
                TEXT("NODE_NOT_FOUND")
            );
        }

        const FMaterialExpressionCollection& Collection = Material->GetExpressionCollection();
        for (UMaterialExpression* OtherExpr : Collection.Expressions)
        {
            if (OtherExpr && OtherExpr != Expression)
            {
                ClearInputReferences(OtherExpr, Expression);
            }
        }

        Material->GetExpressionCollection().RemoveExpression(Expression);
        Expression->MarkAsGarbage();

        Material->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("deleted"), true);
        Result->SetStringField(TEXT("node_name"), NodeName);

        return FMcpCommandResult::Success(Result);
    }

private:
    UMaterialExpression* FindExpression(UMaterial* Material, const FString& NodeName)
    {
        if (!Material)
        {
            return nullptr;
        }

        const FMaterialExpressionCollection& Collection = Material->GetExpressionCollection();
        for (UMaterialExpression* Expression : Collection.Expressions)
        {
            if (Expression && Expression->GetName() == NodeName)
            {
                return Expression;
            }
        }

        return nullptr;
    }

    void ClearInputReferences(UMaterialExpression* Expr, UMaterialExpression* TargetExpr)
    {
        if (!Expr || !TargetExpr) return;

        for (int32 i = 0; i < 32; ++i)
        {
            FExpressionInput* Input = Expr->GetInput(i);
            if (Input && Input->Expression == TargetExpr)
            {
                Input->Expression = nullptr;
                Input->OutputIndex = 0;
            }
        }
    }
};

REGISTER_MCP_COMMAND(FMcpDeleteMaterialNodeHandler)
