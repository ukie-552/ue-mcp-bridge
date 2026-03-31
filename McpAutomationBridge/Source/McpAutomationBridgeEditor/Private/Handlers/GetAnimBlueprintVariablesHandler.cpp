#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"

class FMcpGetAnimBlueprintVariablesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_anim_blueprint_variables"); }

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

        TArray<TSharedPtr<FJsonValue>> VariablesArray;

        for (const FBPVariableDescription& Var : AnimBP->NewVariables)
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

            VarInfo->SetStringField(TEXT("pin_sub_category"), Var.VarType.PinSubCategory.IsNone() ? TEXT("None") : Var.VarType.PinSubCategory.ToString());

            VariablesArray.Add(MakeShareable(new FJsonValueObject(VarInfo)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);
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
        if (TypeStr == TEXT("linearcolor")) return TEXT("linearcolor");
        if (TypeStr == TEXT("object")) return TEXT("object");
        if (TypeStr == TEXT("class")) return TEXT("class");
        if (TypeStr == TEXT("struct")) return TEXT("struct");
        if (TypeStr == TEXT("enum")) return TEXT("enum");
        if (TypeStr == TEXT("byte")) return TEXT("byte");
        if (TypeStr == TEXT("int64")) return TEXT("int64");

        return TypeStr;
    }
};

REGISTER_MCP_COMMAND(FMcpGetAnimBlueprintVariablesHandler)