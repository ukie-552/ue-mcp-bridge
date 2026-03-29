#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Handlers/McpBlueprintUtils.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpGetBlueprintVariablesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_blueprint_variables"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString BlueprintPath;
        if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        UBlueprint* Blueprint = FMcpBlueprintUtils::LoadBlueprint(BlueprintPath);
        if (!Blueprint)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        TArray<TSharedPtr<FJsonValue>> VariablesArray;

        for (const FBPVariableDescription& Var : Blueprint->NewVariables)
        {
            TSharedPtr<FJsonObject> VarInfo = MakeShareable(new FJsonObject);
            VarInfo->SetStringField(TEXT("variable_name"), Var.VarName.ToString());
            VarInfo->SetStringField(TEXT("variable_type"), GetPinTypeString(Var.VarType.PinCategory));
            VarInfo->SetBoolField(TEXT("is_array"), Var.VarType.ContainerType == EPinContainerType::Array);
            VarInfo->SetStringField(TEXT("default_value"), Var.DefaultValue);

            VarInfo->SetBoolField(TEXT("is_exposed"), (Var.PropertyFlags & CPF_ExposeOnSpawn) != 0);
            VarInfo->SetBoolField(TEXT("is_instance_editable"), (Var.PropertyFlags & CPF_Edit) != 0);
            VarInfo->SetBoolField(TEXT("is_blueprint_readonly"), (Var.PropertyFlags & CPF_BlueprintReadOnly) != 0);

            FString Category = Var.Category.ToString();
            if (!Category.IsEmpty())
            {
                VarInfo->SetStringField(TEXT("category"), Category);
            }

            FString Tooltip;
            if (Var.HasMetaData(TEXT("Tooltip")))
            {
                Tooltip = Var.GetMetaData(TEXT("Tooltip"));
            }
            if (!Tooltip.IsEmpty())
            {
                VarInfo->SetStringField(TEXT("tooltip"), Tooltip);
            }

            VariablesArray.Add(MakeShareable(new FJsonValueObject(VarInfo)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("blueprint_path"), BlueprintPath);
        Result->SetArrayField(TEXT("variables"), VariablesArray);
        Result->SetNumberField(TEXT("variable_count"), VariablesArray.Num());

        return FMcpCommandResult::Success(Result);
    }

private:
    FString GetPinTypeString(const FName& PinCategory)
    {
        FString TypeStr = PinCategory.ToString();
        
        if (TypeStr == TEXT("bool")) return TEXT("bool");
        if (TypeStr == TEXT("int")) return TEXT("int");
        if (TypeStr == TEXT("float")) return TEXT("float");
        if (TypeStr == TEXT("string")) return TEXT("string");
        if (TypeStr == TEXT("name")) return TEXT("name");
        if (TypeStr == TEXT("text")) return TEXT("text");
        if (TypeStr == TEXT("vector")) return TEXT("vector");
        if (TypeStr == TEXT("rotator")) return TEXT("rotator");
        if (TypeStr == TEXT("transform")) return TEXT("transform");
        if (TypeStr == TEXT("color")) return TEXT("color");
        if (TypeStr == TEXT("object")) return TEXT("object");
        if (TypeStr == TEXT("class")) return TEXT("class");
        if (TypeStr == TEXT("struct")) return TEXT("struct");
        if (TypeStr == TEXT("enum")) return TEXT("enum");
        
        return TypeStr;
    }
};

REGISTER_MCP_COMMAND(FMcpGetBlueprintVariablesHandler)
