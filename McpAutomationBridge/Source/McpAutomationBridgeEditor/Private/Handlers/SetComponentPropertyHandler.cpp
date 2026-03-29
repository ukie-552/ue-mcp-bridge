#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Camera/CameraComponent.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "UObject/TextProperty.h"

class FMcpSetComponentPropertyHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_component_property"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString ActorId;
        if (!Params->TryGetStringField(TEXT("actor_id"), ActorId))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'actor_id' parameter"), TEXT("MISSING_PARAM"));
        }

        FString ComponentName;
        if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'component_name' parameter"), TEXT("MISSING_PARAM"));
        }

        FString PropertyName;
        if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'property_name' parameter"), TEXT("MISSING_PARAM"));
        }

        AActor* TargetActor = FindActorById(ActorId);
        if (!TargetActor)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Actor not found: %s"), *ActorId),
                TEXT("ACTOR_NOT_FOUND")
            );
        }

        UActorComponent* TargetComponent = nullptr;
        for (UActorComponent* Component : TargetActor->GetComponents())
        {
            if (Component && Component->GetName() == ComponentName)
            {
                TargetComponent = Component;
                break;
            }
        }

        if (!TargetComponent)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Component not found: %s"), *ComponentName),
                TEXT("COMPONENT_NOT_FOUND")
            );
        }

        bool bSuccess = SetPropertyValue(TargetComponent, PropertyName, Params);

        if (!bSuccess)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to set property: %s"), *PropertyName),
                TEXT("PROPERTY_SET_FAILED")
            );
        }

        TargetComponent->MarkRenderStateDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), true);
        Result->SetStringField(TEXT("actor_id"), ActorId);
        Result->SetStringField(TEXT("component_name"), ComponentName);
        Result->SetStringField(TEXT("property_name"), PropertyName);

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

    bool SetPropertyValue(UActorComponent* Component, const FString& PropertyName, const TSharedPtr<FJsonObject>& Params)
    {
        if (!Component) return false;

        FProperty* Property = Component->GetClass()->FindPropertyByName(*PropertyName);
        if (!Property) return false;

        void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Component);

        if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Property))
        {
            double Value = 0;
            if (Params->TryGetNumberField(TEXT("value"), Value))
            {
                if (NumericProp->IsFloatingPoint())
                {
                    NumericProp->SetFloatingPointPropertyValue(ValuePtr, Value);
                }
                else
                {
                    NumericProp->SetIntPropertyValue(ValuePtr, static_cast<int64>(Value));
                }
                return true;
            }
        }
        else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
        {
            bool Value = false;
            if (Params->TryGetBoolField(TEXT("value"), Value))
            {
                BoolProp->SetPropertyValue(ValuePtr, Value);
                return true;
            }
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
        {
            FString Value;
            if (Params->TryGetStringField(TEXT("value"), Value))
            {
                StrProp->SetPropertyValue(ValuePtr, Value);
                return true;
            }
        }
        else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
        {
            FString Value;
            if (Params->TryGetStringField(TEXT("value"), Value))
            {
                NameProp->SetPropertyValue(ValuePtr, *Value);
                return true;
            }
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
        {
            const TSharedPtr<FJsonObject>* StructObj;
            if (Params->TryGetObjectField(TEXT("value"), StructObj))
            {
                if (StructProp->Struct == TBaseStructure<FVector>::Get())
                {
                    FVector Vec = FVector::ZeroVector;
                    double X = 0, Y = 0, Z = 0;
                    (*StructObj)->TryGetNumberField(TEXT("x"), X);
                    (*StructObj)->TryGetNumberField(TEXT("y"), Y);
                    (*StructObj)->TryGetNumberField(TEXT("z"), Z);
                    Vec = FVector(X, Y, Z);
                    StructProp->CopyCompleteValue(ValuePtr, &Vec);
                    return true;
                }
                else if (StructProp->Struct == TBaseStructure<FRotator>::Get())
                {
                    FRotator Rot = FRotator::ZeroRotator;
                    double Pitch = 0, Yaw = 0, Roll = 0;
                    (*StructObj)->TryGetNumberField(TEXT("pitch"), Pitch);
                    (*StructObj)->TryGetNumberField(TEXT("yaw"), Yaw);
                    (*StructObj)->TryGetNumberField(TEXT("roll"), Roll);
                    Rot = FRotator(Pitch, Yaw, Roll);
                    StructProp->CopyCompleteValue(ValuePtr, &Rot);
                    return true;
                }
                else if (StructProp->Struct == TBaseStructure<FLinearColor>::Get())
                {
                    FLinearColor Color = FLinearColor::White;
                    double R = 1, G = 1, B = 1, A = 1;
                    (*StructObj)->TryGetNumberField(TEXT("r"), R);
                    (*StructObj)->TryGetNumberField(TEXT("g"), G);
                    (*StructObj)->TryGetNumberField(TEXT("b"), B);
                    (*StructObj)->TryGetNumberField(TEXT("a"), A);
                    Color = FLinearColor(R, G, B, A);
                    StructProp->CopyCompleteValue(ValuePtr, &Color);
                    return true;
                }
            }
        }
        else if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Property))
        {
            FString AssetPath;
            if (Params->TryGetStringField(TEXT("value"), AssetPath))
            {
                UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
                if (Asset)
                {
                    ObjProp->SetObjectPropertyValue(ValuePtr, Asset);
                    return true;
                }
            }
        }

        return false;
    }
};

REGISTER_MCP_COMMAND(FMcpSetComponentPropertyHandler)
