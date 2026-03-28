#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"

class FMcpGetDetailsPanelPropertiesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_details_panel_properties"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ObjectType;
        if (!Params->TryGetStringField(TEXT("object_type"), ObjectType))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'object_type' parameter (actor/component)"), TEXT("MISSING_PARAM"));
        }

        FString ObjectId;
        if (!Params->TryGetStringField(TEXT("object_id"), ObjectId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'object_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString Category;
        Params->TryGetStringField(TEXT("category"), Category);

        UObject* TargetObject = nullptr;
        UClass* TargetClass = nullptr;

        if (ObjectType == TEXT("actor"))
        {
            TargetObject = FindActorById(ObjectId);
        }
        else if (ObjectType == TEXT("component"))
        {
            FString ActorId;
            if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
            {
                return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' for component lookup"), TEXT("MISSING_PARAM"));
            }
            TargetObject = FindComponentById(ActorId, ObjectId);
        }
        else if (ObjectType == TEXT("class"))
        {
            TargetClass = LoadClass<UObject>(nullptr, *ObjectId);
        }

        if (!TargetObject && !TargetClass)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Object not found: %s"), *ObjectId),
                TEXT("OBJECT_NOT_FOUND")
            );
        }

        UClass* ClassToInspect = TargetObject ? TargetObject->GetClass() : TargetClass;

        TArray<TSharedPtr<FJsonValue>> PropertiesArray;
        TArray<TSharedPtr<FJsonValue>> CategoriesArray;
        TSet<FString> FoundCategories;

        for (TFieldIterator<FProperty> PropIt(ClassToInspect); PropIt; ++PropIt)
        {
            FProperty* Property = *PropIt;
            
            if (!Property->HasAnyPropertyFlags(CPF_Edit))
            {
                continue;
            }

            FString PropertyCategory = Property->GetMetaData(TEXT("Category"));
            if (PropertyCategory.IsEmpty())
            {
                PropertyCategory = TEXT("Default");
            }

            if (!Category.IsEmpty() && PropertyCategory != Category)
            {
                continue;
            }

            if (!FoundCategories.Contains(PropertyCategory))
            {
                FoundCategories.Add(PropertyCategory);
                CategoriesArray.Add(MakeShareable(new FJsonValueString(PropertyCategory)));
            }

            TSharedPtr<FJsonObject> PropInfo = MakeShareable(new FJsonObject);
            PropInfo->SetStringField(TEXT("name"), Property->GetName());
            PropInfo->SetStringField(TEXT("display_name"), Property->GetDisplayNameText().ToString());
            PropInfo->SetStringField(TEXT("category"), PropertyCategory);
            PropInfo->SetStringField(TEXT("type"), Property->GetClass()->GetName());
            PropInfo->SetStringField(TEXT("tooltip"), Property->GetToolTipText().ToString());
            PropInfo->SetBoolField(TEXT("is_read_only"), Property->HasAnyPropertyFlags(CPF_EditConst));
            PropInfo->SetBoolField(TEXT("is_array"), Property->IsA<FArrayProperty>());
            PropInfo->SetBoolField(TEXT("is_struct"), Property->IsA<FStructProperty>());

            if (TargetObject)
            {
                FString CurrentValue = GetPropertyValueAsString(Property, TargetObject);
                PropInfo->SetStringField(TEXT("current_value"), CurrentValue);
            }

            PropertiesArray.Add(MakeShareable(new FJsonValueObject(PropInfo)));
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetStringField(TEXT("object_type"), ObjectType);
        Result->SetStringField(TEXT("object_id"), ObjectId);
        Result->SetStringField(TEXT("class_name"), ClassToInspect->GetName());
        Result->SetArrayField(TEXT("properties"), PropertiesArray);
        Result->SetArrayField(TEXT("categories"), CategoriesArray);
        Result->SetNumberField(TEXT("property_count"), PropertiesArray.Num());

        return FMcpCommandResult::Success(Result);
    }

private:
    AActor* FindActorById(const FString& ActorId)
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World) return nullptr;

        FGuid TargetGuid;
        if (!FGuid::Parse(ActorId, TargetGuid)) return nullptr;

        for (TActorIterator<AActor> It(World); It; ++It)
        {
            if ((*It)->GetActorGuid() == TargetGuid)
            {
                return *It;
            }
        }

        return nullptr;
    }

    UActorComponent* FindComponentById(const FString& ActorId, const FString& ComponentName)
    {
        AActor* Actor = FindActorById(ActorId);
        if (!Actor) return nullptr;

        for (UActorComponent* Component : Actor->GetComponents())
        {
            if (Component && Component->GetName() == ComponentName)
            {
                return Component;
            }
        }

        return nullptr;
    }

    FString GetPropertyValueAsString(FProperty* Property, UObject* Object)
    {
        if (!Property || !Object) return TEXT("");

        void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);

        if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Property))
        {
            if (NumericProp->IsFloatingPoint())
            {
                return FString::SanitizeFloat(NumericProp->GetFloatingPointPropertyValue(ValuePtr));
            }
            else
            {
                return FString::Printf(TEXT("%lld"), NumericProp->GetIntPropertyValue(ValuePtr));
            }
        }
        else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
        {
            return BoolProp->GetPropertyValue(ValuePtr) ? TEXT("true") : TEXT("false");
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
        {
            return StrProp->GetPropertyValue(ValuePtr);
        }
        else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
        {
            return NameProp->GetPropertyValue(ValuePtr).ToString();
        }
        else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
        {
            return EnumProp->GetEnum()->GetNameStringByValue(EnumProp->GetUnderlyingProperty()->GetIntPropertyValue(ValuePtr));
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
        {
            if (StructProp->Struct == TBaseStructure<FVector>::Get())
            {
                FVector* Vec = static_cast<FVector*>(ValuePtr);
                return FString::Printf(TEXT("(X=%f, Y=%f, Z=%f)"), Vec->X, Vec->Y, Vec->Z);
            }
            else if (StructProp->Struct == TBaseStructure<FRotator>::Get())
            {
                FRotator* Rot = static_cast<FRotator*>(ValuePtr);
                return FString::Printf(TEXT("(P=%f, Y=%f, R=%f)"), Rot->Pitch, Rot->Yaw, Rot->Roll);
            }
            else if (StructProp->Struct == TBaseStructure<FLinearColor>::Get())
            {
                FLinearColor* Color = static_cast<FLinearColor*>(ValuePtr);
                return FString::Printf(TEXT("(R=%f, G=%f, B=%f, A=%f)"), Color->R, Color->G, Color->B, Color->A);
            }
        }

        return TEXT("(complex type)");
    }
};

REGISTER_MCP_COMMAND(FMcpGetDetailsPanelPropertiesHandler)
