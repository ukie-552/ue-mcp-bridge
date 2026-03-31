#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphSchema_K2.h"

class FMcpCreateAnimBlueprintVariableHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_anim_blueprint_variable"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString AnimBlueprintPath;
        if (!Params->TryGetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'anim_blueprint_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString VariableName;
        if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'variable_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString VariableType = TEXT("bool");
        Params->TryGetStringField(TEXT("variable_type"), VariableType);

        bool bIsArray = false;
        Params->TryGetBoolField(TEXT("is_array"), bIsArray);

        FString DefaultValue;
        Params->TryGetStringField(TEXT("default_value"), DefaultValue);

        FString Category = TEXT("Default");
        Params->TryGetStringField(TEXT("category"), Category);

        bool bIsExposed = false;
        Params->TryGetBoolField(TEXT("is_exposed"), bIsExposed);

        UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
        if (!AnimBP)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Animation Blueprint not found: %s"), *AnimBlueprintPath),
                TEXT("ASSET_NOT_FOUND")
            );
        }

        FEdGraphPinType PinType = GetPinTypeFromString(VariableType, bIsArray);

        bool bSuccess = FBlueprintEditorUtils::AddMemberVariable(
            AnimBP,
            *VariableName,
            PinType
        );

        if (!bSuccess)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create variable: %s"), *VariableName),
                TEXT("VARIABLE_CREATION_FAILED")
            );
        }

        FBlueprintEditorUtils::MarkBlueprintAsModified(AnimBP);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("variable_name"), VariableName);
        Result->SetStringField(TEXT("variable_type"), VariableType);
        Result->SetBoolField(TEXT("is_array"), bIsArray);
        Result->SetStringField(TEXT("category"), Category);
        Result->SetBoolField(TEXT("is_exposed"), bIsExposed);
        Result->SetStringField(TEXT("anim_blueprint_path"), AnimBlueprintPath);

        return FMcpCommandResult::Success(Result);
    }

private:
    FEdGraphPinType GetPinTypeFromString(const FString& TypeString, bool bIsArray)
    {
        FEdGraphPinType PinType;

        if (TypeString == TEXT("bool") || TypeString == TEXT("Boolean"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
        }
        else if (TypeString == TEXT("int") || TypeString == TEXT("Integer"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
        }
        else if (TypeString == TEXT("float") || TypeString == TEXT("Float"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
            PinType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
        }
        else if (TypeString == TEXT("double"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
            PinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
        }
        else if (TypeString == TEXT("string") || TypeString == TEXT("String"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_String;
        }
        else if (TypeString == TEXT("name") || TypeString == TEXT("Name"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Name;
        }
        else if (TypeString == TEXT("text") || TypeString == TEXT("Text"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Text;
        }
        else if (TypeString == TEXT("vector") || TypeString == TEXT("Vector"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
        }
        else if (TypeString == TEXT("rotator") || TypeString == TEXT("Rotator"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            PinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
        }
        else if (TypeString == TEXT("transform") || TypeString == TEXT("Transform"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            PinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
        }
        else if (TypeString == TEXT("color") || TypeString == TEXT("LinearColor"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            PinType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();
        }
        else if (TypeString == TEXT("object") || TypeString == TEXT("Object"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
            PinType.PinSubCategoryObject = UObject::StaticClass();
        }
        else if (TypeString == TEXT("class") || TypeString == TEXT("Class"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Class;
            PinType.PinSubCategoryObject = UObject::StaticClass();
        }
        else if (TypeString == TEXT("pose") || TypeString == TEXT("Pose"))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            PinType.PinSubCategoryObject = FPoseLink::StaticStruct();
        }
        else
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Object;

            UClass* CustomClass = LoadClass<UObject>(nullptr, *TypeString);
            if (CustomClass)
            {
                PinType.PinSubCategoryObject = CustomClass;
            }
        }

        if (bIsArray)
        {
            PinType.ContainerType = EPinContainerType::Array;
        }

        return PinType;
    }
};

REGISTER_MCP_COMMAND(FMcpCreateAnimBlueprintVariableHandler)