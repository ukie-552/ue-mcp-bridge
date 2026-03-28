#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Editor.h"

class FMcpSetActorPropertyHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_actor_property"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString PropertyName;
        if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'property_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString PropertyValue;
        if (!Params->TryGetStringField(TEXT("property_value"), PropertyValue))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'property_value' parameter"), TEXT("MISSING_PARAM"));
        }

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        FProperty* Property = TargetActor->GetClass()->FindPropertyByName(*PropertyName);
        if (!Property)
        {
            for (UActorComponent* Component : TargetActor->GetComponents())
            {
                if (Component)
                {
                    Property = Component->GetClass()->FindPropertyByName(*PropertyName);
                    if (Property)
                    {
                        TargetActor->Modify();
                        SetPropertyFromString(Property, Component, PropertyValue);
                        
                        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
                        Result->SetBoolField(TEXT("set"), true);
                        Result->SetStringField(TEXT("actor_id"), ActorId);
                        Result->SetStringField(TEXT("property_name"), PropertyName);
                        Result->SetStringField(TEXT("property_value"), PropertyValue);
                        Result->SetStringField(TEXT("component_name"), Component->GetName());
                        
                        return FMcpCommandResult::Success(Result);
                    }
                }
            }

            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Property not found: %s"), *PropertyName),
                TEXT("PROPERTY_NOT_FOUND")
            );
        }

        TargetActor->Modify();
        SetPropertyFromString(Property, TargetActor, PropertyValue);

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), true);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("property_name"), PropertyName);
        Result->SetStringField(TEXT("property_value"), PropertyValue);

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

    void SetPropertyFromString(FProperty* Property, UObject* Object, const FString& Value)
    {
        if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
        {
            BoolProp->SetPropertyValue_InContainer(Object, Value.ToLower() == TEXT("true") || Value == TEXT("1"));
        }
        else if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
        {
            IntProp->SetPropertyValue_InContainer(Object, FCString::Atoi(*Value));
        }
        else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
        {
            FloatProp->SetPropertyValue_InContainer(Object, FCString::Atof(*Value));
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
        {
            StrProp->SetPropertyValue_InContainer(Object, Value);
        }
        else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
        {
            NameProp->SetPropertyValue_InContainer(Object, *Value);
        }
        else if (FTextProperty* TextProp = CastField<FTextProperty>(Property))
        {
            TextProp->SetPropertyValue_InContainer(Object, FText::FromString(Value));
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
        {
            if (StructProp->Struct == TBaseStructure<FVector>::Get())
            {
                FVector Vector;
                Vector.InitFromString(Value);
                StructProp->SetValue_InContainer(Object, &Vector);
            }
            else if (StructProp->Struct == TBaseStructure<FRotator>::Get())
            {
                FRotator Rotator;
                Rotator.InitFromString(Value);
                StructProp->SetValue_InContainer(Object, &Rotator);
            }
            else if (StructProp->Struct == TBaseStructure<FLinearColor>::Get())
            {
                FLinearColor Color;
                Color.InitFromString(Value);
                StructProp->SetValue_InContainer(Object, &Color);
            }
        }
    }
};

REGISTER_MCP_COMMAND(FMcpSetActorPropertyHandler)
