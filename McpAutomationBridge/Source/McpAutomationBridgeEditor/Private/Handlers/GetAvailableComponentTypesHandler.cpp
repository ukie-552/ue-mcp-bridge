#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/LightComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"

struct FComponentInfo
{
    FString Name;
    FString Description;
};

struct FComponentCategory
{
    FString CategoryName;
    TArray<FComponentInfo> Components;
};

class FMcpGetAvailableComponentTypesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_available_component_types"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Category;
        Params->TryGetStringField(TEXT("category"), Category);

        TArray<FComponentCategory> ComponentCategories = BuildComponentCategories();

        TArray<TSharedPtr<FJsonValue>> ComponentsArray;

        for (const FComponentCategory& Cat : ComponentCategories)
        {
            if (Category.IsEmpty() || Category == Cat.CategoryName)
            {
                for (const FComponentInfo& Comp : Cat.Components)
                {
                    TSharedPtr<FJsonObject> CompInfo = MakeShareable(new FJsonObject);
                    CompInfo->SetStringField(TEXT("name"), Comp.Name);
                    CompInfo->SetStringField(TEXT("category"), Cat.CategoryName);
                    CompInfo->SetStringField(TEXT("description"), Comp.Description);
                    ComponentsArray.Add(MakeShareable(new FJsonValueObject(CompInfo)));
                }
            }
        }

        TArray<UClass*> AllComponentClasses;
        GetDerivedClasses(UActorComponent::StaticClass(), AllComponentClasses);

        TSet<FString> AlreadyAddedTypes;
        for (const auto& JsonValue : ComponentsArray)
        {
            AlreadyAddedTypes.Add(JsonValue->AsObject()->GetStringField(TEXT("name")));
        }

        for (UClass* CompClass : AllComponentClasses)
        {
            if (CompClass && !CompClass->HasAnyClassFlags(CLASS_Abstract))
            {
                FString ClassName = CompClass->GetName();
                if (!AlreadyAddedTypes.Contains(ClassName))
                {
                    TSharedPtr<FJsonObject> CompInfo = MakeShareable(new FJsonObject);
                    CompInfo->SetStringField(TEXT("name"), ClassName);
                    CompInfo->SetStringField(TEXT("category"), TEXT("All"));
                    CompInfo->SetStringField(TEXT("class_path"), CompClass->GetPathName());
                    ComponentsArray.Add(MakeShareable(new FJsonValueObject(CompInfo)));
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("component_types"), ComponentsArray);
        Result->SetNumberField(TEXT("count"), ComponentsArray.Num());

        return FMcpCommandResult::Success(Result);
    }

private:
    TArray<FComponentCategory> BuildComponentCategories()
    {
        TArray<FComponentCategory> Categories;

        FComponentCategory Rendering;
        Rendering.CategoryName = TEXT("Rendering");
        Rendering.Components.Add({TEXT("StaticMeshComponent"), TEXT("Renders a static mesh")});
        Rendering.Components.Add({TEXT("SkeletalMeshComponent"), TEXT("Renders a skeletal mesh with animation")});
        Rendering.Components.Add({TEXT("InstancedStaticMeshComponent"), TEXT("Renders multiple instances of a mesh")});
        Rendering.Components.Add({TEXT("HierarchicalInstancedStaticMeshComponent"), TEXT("Hierarchical instanced mesh rendering")});
        Rendering.Components.Add({TEXT("NiagaraComponent"), TEXT("Niagara particle system component")});
        Rendering.Components.Add({TEXT("ParticleSystemComponent"), TEXT("Cascade particle system component")});
        Rendering.Components.Add({TEXT("WidgetComponent"), TEXT("Renders UMG widget in 3D space")});
        Rendering.Components.Add({TEXT("TextRenderComponent"), TEXT("Renders text in 3D space")});
        Categories.Add(Rendering);

        FComponentCategory Lights;
        Lights.CategoryName = TEXT("Lights");
        Lights.Components.Add({TEXT("DirectionalLightComponent"), TEXT("Directional light source")});
        Lights.Components.Add({TEXT("PointLightComponent"), TEXT("Point light source")});
        Lights.Components.Add({TEXT("SpotLightComponent"), TEXT("Spotlight source")});
        Lights.Components.Add({TEXT("RectLightComponent"), TEXT("Rectangular area light")});
        Lights.Components.Add({TEXT("SkyLightComponent"), TEXT("Sky light for environment")});
        Categories.Add(Lights);

        FComponentCategory Camera;
        Camera.CategoryName = TEXT("Camera");
        Camera.Components.Add({TEXT("CameraComponent"), TEXT("Camera for rendering view")});
        Camera.Components.Add({TEXT("SpringArmComponent"), TEXT("Spring arm for camera follow")});
        Camera.Components.Add({TEXT("CineCameraComponent"), TEXT("Cinematic camera with advanced settings")});
        Categories.Add(Camera);

        FComponentCategory Physics;
        Physics.CategoryName = TEXT("Physics");
        Physics.Components.Add({TEXT("BoxComponent"), TEXT("Box collision shape")});
        Physics.Components.Add({TEXT("SphereComponent"), TEXT("Sphere collision shape")});
        Physics.Components.Add({TEXT("CapsuleComponent"), TEXT("Capsule collision shape")});
        Physics.Components.Add({TEXT("StaticMeshComponent"), TEXT("Static mesh with collision")});
        Categories.Add(Physics);

        FComponentCategory Audio;
        Audio.CategoryName = TEXT("Audio");
        Audio.Components.Add({TEXT("AudioComponent"), TEXT("Plays audio in 3D space")});
        Audio.Components.Add({TEXT("AudioComponent2"), TEXT("Audio component with advanced features")});
        Categories.Add(Audio);

        FComponentCategory Movement;
        Movement.CategoryName = TEXT("Movement");
        Movement.Components.Add({TEXT("MovementComponent"), TEXT("Base movement component")});
        Movement.Components.Add({TEXT("ProjectileMovementComponent"), TEXT("Projectile movement")});
        Movement.Components.Add({TEXT("RotatingMovementComponent"), TEXT("Rotating movement")});
        Movement.Components.Add({TEXT("FloatingPawnMovement"), TEXT("Floating pawn movement")});
        Movement.Components.Add({TEXT("CharacterMovementComponent"), TEXT("Character movement with walking/flying")});
        Movement.Components.Add({TEXT("FlyingMovementComponent"), TEXT("Flying movement")});
        Categories.Add(Movement);

        FComponentCategory AI;
        AI.CategoryName = TEXT("AI");
        AI.Components.Add({TEXT("PawnSensingComponent"), TEXT("Pawn sensing for AI")});
        AI.Components.Add({TEXT("AIPerceptionComponent"), TEXT("AI perception system")});
        AI.Components.Add({TEXT("PawnNoiseEmitterComponent"), TEXT("Emits noise for AI detection")});
        Categories.Add(AI);

        FComponentCategory Effects;
        Effects.CategoryName = TEXT("Effects");
        Effects.Components.Add({TEXT("NiagaraComponent"), TEXT("Niagara VFX system")});
        Effects.Components.Add({TEXT("ParticleSystemComponent"), TEXT("Cascade particle system")});
        Effects.Components.Add({TEXT("ExponentialHeightFogComponent"), TEXT("Height-based fog")});
        Effects.Components.Add({TEXT("AtmosphericFogComponent"), TEXT("Atmospheric fog")});
        Categories.Add(Effects);

        FComponentCategory Utility;
        Utility.CategoryName = TEXT("Utility");
        Utility.Components.Add({TEXT("SceneComponent"), TEXT("Base component with transform")});
        Utility.Components.Add({TEXT("ActorComponent"), TEXT("Base component without transform")});
        Utility.Components.Add({TEXT("BillboardComponent"), TEXT("Always-facing billboard sprite")});
        Utility.Components.Add({TEXT("ArrowComponent"), TEXT("Debug arrow visualization")});
        Utility.Components.Add({TEXT("DebugDrawComponent"), TEXT("Debug drawing component")});
        Categories.Add(Utility);

        return Categories;
    }

    void GetDerivedClasses(UClass* BaseClass, TArray<UClass*>& DerivedClasses)
    {
        if (!BaseClass) return;

        TArray<UClass*> AllClasses;
        for (TObjectIterator<UClass> It; It; ++It)
        {
            UClass* Class = *It;
            if (Class->IsChildOf(BaseClass) && !Class->HasAnyClassFlags(CLASS_Abstract))
            {
                DerivedClasses.AddUnique(Class);
            }
        }
    }
};

REGISTER_MCP_COMMAND(FMcpGetAvailableComponentTypesHandler)
