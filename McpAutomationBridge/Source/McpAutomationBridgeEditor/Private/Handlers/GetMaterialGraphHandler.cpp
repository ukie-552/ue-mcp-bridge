#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"

class FMcpGetMaterialGraphHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_material_graph"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString MaterialPath;
        if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'material_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UMaterial* Material = LoadObject<UMaterial>(nullptr, *MaterialPath);
        if (!Material)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Material not found: %s"), *MaterialPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> NodesArray;

        const FMaterialExpressionCollection& Collection = Material->GetExpressionCollection();
        for (UMaterialExpression* Expression : Collection.Expressions)
        {
            if (Expression)
            {
                NodesArray.Add(ExpressionToJson(Expression));
            }
        }

        TArray<TSharedPtr<FJsonValue>> ConnectionsArray;
        GatherConnections(Material, ConnectionsArray);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("material_name"), Material->GetName());
        Result->SetStringField(TEXT("material_path"), MaterialPath);
        Result->SetArrayField(TEXT("nodes"), NodesArray);
        Result->SetArrayField(TEXT("connections"), ConnectionsArray);
        Result->SetNumberField(TEXT("node_count"), NodesArray.Num());

        TSharedPtr<FJsonObject> PropertiesObj = MakeShareable(new FJsonObject);
        PropertiesObj->SetStringField(TEXT("blend_mode"), BlendModeToString(Material->BlendMode));
        PropertiesObj->SetBoolField(TEXT("two_sided"), Material->TwoSided);
        Result->SetObjectField(TEXT("properties"), PropertiesObj);

        return FMcpCommandResult::Success(Result);
    }

private:
    TSharedPtr<FJsonValue> ExpressionToJson(UMaterialExpression* Expression)
    {
        TSharedPtr<FJsonObject> NodeObj = MakeShareable(new FJsonObject);
        NodeObj->SetStringField(TEXT("name"), Expression->GetName());
        NodeObj->SetStringField(TEXT("type"), Expression->GetClass()->GetName());
        NodeObj->SetNumberField(TEXT("pos_x"), Expression->MaterialExpressionEditorX);
        NodeObj->SetNumberField(TEXT("pos_y"), Expression->MaterialExpressionEditorY);

        TArray<TSharedPtr<FJsonValue>> InputsArray;

        for (int32 i = 0; i < 32; ++i)
        {
            FExpressionInput* Input = Expression->GetInput(i);
            if (Input)
            {
                TSharedPtr<FJsonObject> InputObj = MakeShareable(new FJsonObject);
                InputObj->SetNumberField(TEXT("index"), i);
                InputObj->SetStringField(TEXT("name"), Expression->GetInputName(i).ToString());

                if (Input->Expression)
                {
                    InputObj->SetStringField(TEXT("connected_node"), Input->Expression->GetName());
                    InputObj->SetNumberField(TEXT("output_index"), Input->OutputIndex);
                }

                InputsArray.Add(MakeShareable(new FJsonValueObject(InputObj)));
            }
        }
        NodeObj->SetArrayField(TEXT("inputs"), InputsArray);

        TArray<TSharedPtr<FJsonValue>> OutputsArray;
        TArray<FExpressionOutput> Outputs = Expression->GetOutputs();

        for (int32 i = 0; i < Outputs.Num(); ++i)
        {
            TSharedPtr<FJsonObject> OutputObj = MakeShareable(new FJsonObject);
            OutputObj->SetNumberField(TEXT("index"), i);
            OutputObj->SetStringField(TEXT("name"), Outputs[i].OutputName.ToString());
            OutputObj->SetStringField(TEXT("description"), Outputs[i].OutputName.ToString());
            OutputsArray.Add(MakeShareable(new FJsonValueObject(OutputObj)));
        }
        NodeObj->SetArrayField(TEXT("outputs"), OutputsArray);

        return MakeShareable(new FJsonValueObject(NodeObj));
    }

    void GatherConnections(UMaterial* Material, TArray<TSharedPtr<FJsonValue>>& OutConnections)
    {
        const FMaterialExpressionCollection& Collection = Material->GetExpressionCollection();
        for (UMaterialExpression* Expression : Collection.Expressions)
        {
            if (!Expression)
            {
                continue;
            }

            for (int32 i = 0; i < 32; ++i)
            {
                FExpressionInput* Input = Expression->GetInput(i);
                if (Input && Input->Expression)
                {
                    TSharedPtr<FJsonObject> ConnObj = MakeShareable(new FJsonObject);
                    ConnObj->SetStringField(TEXT("source_node"), Input->Expression->GetName());
                    ConnObj->SetNumberField(TEXT("source_output"), Input->OutputIndex);
                    ConnObj->SetStringField(TEXT("target_node"), Expression->GetName());
                    ConnObj->SetNumberField(TEXT("target_input"), i);
                    OutConnections.Add(MakeShareable(new FJsonValueObject(ConnObj)));
                }
            }
        }
    }

    FString BlendModeToString(EBlendMode BlendMode)
    {
        switch (BlendMode)
        {
        case BLEND_Opaque:
            return TEXT("Opaque");
        case BLEND_Masked:
            return TEXT("Masked");
        case BLEND_Translucent:
            return TEXT("Translucent");
        case BLEND_Additive:
            return TEXT("Additive");
        case BLEND_Modulate:
            return TEXT("Modulate");
        case BLEND_AlphaComposite:
            return TEXT("AlphaComposite");
        default:
            return TEXT("Unknown");
        }
    }
};

REGISTER_MCP_COMMAND(FMcpGetMaterialGraphHandler)
