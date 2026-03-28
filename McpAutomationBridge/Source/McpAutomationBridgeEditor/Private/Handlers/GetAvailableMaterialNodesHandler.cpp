#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionPower.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionAbs.h"
#include "Materials/MaterialExpressionSine.h"
#include "Materials/MaterialExpressionCosine.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionFrac.h"
#include "Materials/MaterialExpressionFloor.h"
#include "Materials/MaterialExpressionCeil.h"
#include "Materials/MaterialExpressionRound.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionViewSize.h"
#include "Materials/MaterialExpressionPixelNormalWS.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionCameraVectorWS.h"
#include "Materials/MaterialExpressionReflectionVectorWS.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionPanner.h"
#include "Materials/MaterialExpressionRotator.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionDesaturation.h"
#include "Materials/MaterialExpressionDepthFade.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionNoise.h"
#include "Materials/MaterialExpressionBumpOffset.h"
#include "Materials/MaterialExpressionDistance.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionCrossProduct.h"
#include "Materials/MaterialExpressionNormalize.h"
#include "Materials/MaterialExpressionLength.h"
#include "Materials/MaterialExpressionObjectPositionWS.h"
#include "Materials/MaterialExpressionObjectRadius.h"
#include "Materials/MaterialExpressionObjectLocalBounds.h"
#include "Materials/MaterialExpressionParticleColor.h"
#include "Materials/MaterialExpressionParticlePositionWS.h"
#include "Materials/MaterialExpressionParticleRadius.h"
#include "Materials/MaterialExpressionParticleSpeed.h"
#include "Materials/MaterialExpressionVertexColor.h"

struct FMaterialNodeInfo
{
    FString Name;
    FString Description;
};

struct FMaterialNodeCategory
{
    FString CategoryName;
    TArray<FMaterialNodeInfo> Nodes;
};

class FMcpGetAvailableMaterialNodesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_available_material_nodes"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Category;
        Params->TryGetStringField(TEXT("category"), Category);

        FString SearchTerm;
        Params->TryGetStringField(TEXT("search"), SearchTerm);

        TArray<FMaterialNodeCategory> NodeCategories = BuildNodeCategories();

        TArray<TSharedPtr<FJsonValue>> NodesArray;

        for (const FMaterialNodeCategory& Cat : NodeCategories)
        {
            if (Category.IsEmpty() || Category == Cat.CategoryName)
            {
                for (const FMaterialNodeInfo& Node : Cat.Nodes)
                {
                    if (SearchTerm.IsEmpty() || Node.Name.Contains(SearchTerm) || Node.Description.Contains(SearchTerm))
                    {
                        TSharedPtr<FJsonObject> NodeInfo = MakeShareable(new FJsonObject);
                        NodeInfo->SetStringField(TEXT("name"), Node.Name);
                        NodeInfo->SetStringField(TEXT("category"), Cat.CategoryName);
                        NodeInfo->SetStringField(TEXT("description"), Node.Description);
                        NodesArray.Add(MakeShareable(new FJsonValueObject(NodeInfo)));
                    }
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("nodes"), NodesArray);
        Result->SetNumberField(TEXT("count"), NodesArray.Num());

        TArray<TSharedPtr<FJsonValue>> CategoriesArray;
        for (const FMaterialNodeCategory& Cat : NodeCategories)
        {
            CategoriesArray.Add(MakeShareable(new FJsonValueString(Cat.CategoryName)));
        }
        Result->SetArrayField(TEXT("categories"), CategoriesArray);

        return FMcpCommandResult::Success(Result);
    }

private:
    TArray<FMaterialNodeCategory> BuildNodeCategories()
    {
        TArray<FMaterialNodeCategory> Categories;

        FMaterialNodeCategory Constants;
        Constants.CategoryName = TEXT("Constants");
        Constants.Nodes.Add({TEXT("Constant"), TEXT("Single float constant value")});
        Constants.Nodes.Add({TEXT("Constant2Vector"), TEXT("Two-component vector constant")});
        Constants.Nodes.Add({TEXT("Constant3Vector"), TEXT("Three-component vector constant (RGB)")});
        Constants.Nodes.Add({TEXT("Constant4Vector"), TEXT("Four-component vector constant (RGBA)")});
        Categories.Add(Constants);

        FMaterialNodeCategory Parameters;
        Parameters.CategoryName = TEXT("Parameters");
        Parameters.Nodes.Add({TEXT("ScalarParameter"), TEXT("Float parameter exposed to material instance")});
        Parameters.Nodes.Add({TEXT("VectorParameter"), TEXT("Vector parameter exposed to material instance")});
        Parameters.Nodes.Add({TEXT("TextureObjectParameter"), TEXT("Texture parameter exposed to material instance")});
        Parameters.Nodes.Add({TEXT("FontSampleParameter"), TEXT("Font sample parameter")});
        Parameters.Nodes.Add({TEXT("StaticBoolParameter"), TEXT("Static boolean parameter")});
        Parameters.Nodes.Add({TEXT("StaticSwitchParameter"), TEXT("Static switch parameter")});
        Categories.Add(Parameters);

        FMaterialNodeCategory Texture;
        Texture.CategoryName = TEXT("Texture");
        Texture.Nodes.Add({TEXT("TextureSample"), TEXT("Samples a texture")});
        Texture.Nodes.Add({TEXT("TextureObject"), TEXT("Texture object reference")});
        Texture.Nodes.Add({TEXT("TextureCoordinate"), TEXT("UV texture coordinates")});
        Texture.Nodes.Add({TEXT("TextureProperty"), TEXT("Texture property access")});
        Categories.Add(Texture);

        FMaterialNodeCategory Math;
        Math.CategoryName = TEXT("Math");
        Math.Nodes.Add({TEXT("Add"), TEXT("Adds two values")});
        Math.Nodes.Add({TEXT("Subtract"), TEXT("Subtracts two values")});
        Math.Nodes.Add({TEXT("Multiply"), TEXT("Multiplies two values")});
        Math.Nodes.Add({TEXT("Divide"), TEXT("Divides two values")});
        Math.Nodes.Add({TEXT("Power"), TEXT("Raises to power")});
        Math.Nodes.Add({TEXT("Lerp"), TEXT("Linear interpolation")});
        Math.Nodes.Add({TEXT("Clamp"), TEXT("Clamps value to range")});
        Math.Nodes.Add({TEXT("Saturate"), TEXT("Clamps to 0-1 range")});
        Math.Nodes.Add({TEXT("Abs"), TEXT("Absolute value")});
        Math.Nodes.Add({TEXT("Sine"), TEXT("Sine function")});
        Math.Nodes.Add({TEXT("Cosine"), TEXT("Cosine function")});
        Math.Nodes.Add({TEXT("Frac"), TEXT("Fractional part")});
        Math.Nodes.Add({TEXT("Floor"), TEXT("Floor function")});
        Math.Nodes.Add({TEXT("Ceil"), TEXT("Ceiling function")});
        Math.Nodes.Add({TEXT("Round"), TEXT("Round to nearest integer")});
        Math.Nodes.Add({TEXT("OneMinus"), TEXT("One minus value (1-x)")});
        Math.Nodes.Add({TEXT("FWidth"), TEXT("Filter width")});
        Categories.Add(Math);

        FMaterialNodeCategory VectorOps;
        VectorOps.CategoryName = TEXT("Vector Operations");
        VectorOps.Nodes.Add({TEXT("AppendVector"), TEXT("Appends vectors together")});
        VectorOps.Nodes.Add({TEXT("ComponentMask"), TEXT("Selects specific components")});
        VectorOps.Nodes.Add({TEXT("DotProduct"), TEXT("Dot product of two vectors")});
        VectorOps.Nodes.Add({TEXT("CrossProduct"), TEXT("Cross product of two vectors")});
        VectorOps.Nodes.Add({TEXT("Normalize"), TEXT("Normalizes a vector")});
        VectorOps.Nodes.Add({TEXT("Length"), TEXT("Vector length")});
        VectorOps.Nodes.Add({TEXT("Distance"), TEXT("Distance between two points")});
        VectorOps.Nodes.Add({TEXT("Reflect"), TEXT("Reflects vector about normal")});
        Categories.Add(VectorOps);

        FMaterialNodeCategory UV;
        UV.CategoryName = TEXT("UV");
        UV.Nodes.Add({TEXT("Panner"), TEXT("Pans UV coordinates over time")});
        UV.Nodes.Add({TEXT("Rotator"), TEXT("Rotates UV coordinates")});
        UV.Nodes.Add({TEXT("Scale"), TEXT("Scales UV coordinates")});
        UV.Nodes.Add({TEXT("BumpOffset"), TEXT("Parallax bump offset")});
        Categories.Add(UV);

        FMaterialNodeCategory World;
        World.CategoryName = TEXT("World");
        World.Nodes.Add({TEXT("WorldPosition"), TEXT("World space position")});
        World.Nodes.Add({TEXT("ObjectPosition"), TEXT("Object world position")});
        World.Nodes.Add({TEXT("ObjectRadius"), TEXT("Object radius")});
        World.Nodes.Add({TEXT("ObjectLocalBounds"), TEXT("Object local bounds")});
        World.Nodes.Add({TEXT("ObjectWorldBounds"), TEXT("Object world bounds")});
        World.Nodes.Add({TEXT("VertexNormal"), TEXT("Vertex normal in world space")});
        World.Nodes.Add({TEXT("PixelNormal"), TEXT("Pixel normal")});
        World.Nodes.Add({TEXT("CameraVector"), TEXT("Camera direction vector")});
        World.Nodes.Add({TEXT("ReflectionVector"), TEXT("Reflection vector")});
        World.Nodes.Add({TEXT("ViewSize"), TEXT("View size")});
        Categories.Add(World);

        FMaterialNodeCategory Effects;
        Effects.CategoryName = TEXT("Effects");
        Effects.Nodes.Add({TEXT("Fresnel"), TEXT("Fresnel effect")});
        Effects.Nodes.Add({TEXT("DepthFade"), TEXT("Depth fade for transparency")});
        Effects.Nodes.Add({TEXT("Noise"), TEXT("Procedural noise")});
        Effects.Nodes.Add({TEXT("Desaturation"), TEXT("Desaturates colors")});
        Categories.Add(Effects);

        FMaterialNodeCategory Particles;
        Particles.CategoryName = TEXT("Particles");
        Particles.Nodes.Add({TEXT("ParticleColor"), TEXT("Particle color")});
        Particles.Nodes.Add({TEXT("ParticlePosition"), TEXT("Particle world position")});
        Particles.Nodes.Add({TEXT("ParticleRadius"), TEXT("Particle radius")});
        Particles.Nodes.Add({TEXT("ParticleSpeed"), TEXT("Particle speed")});
        Categories.Add(Particles);

        FMaterialNodeCategory Vertex;
        Vertex.CategoryName = TEXT("Vertex");
        Vertex.Nodes.Add({TEXT("VertexColor"), TEXT("Vertex color")});
        Vertex.Nodes.Add({TEXT("WorldPosition"), TEXT("World position")});
        Vertex.Nodes.Add({TEXT("VertexNormal"), TEXT("Vertex normal")});
        Categories.Add(Vertex);

        FMaterialNodeCategory Time;
        Time.CategoryName = TEXT("Time");
        Time.Nodes.Add({TEXT("Time"), TEXT("Game time")});
        Time.Nodes.Add({TEXT("Sine"), TEXT("Sine of time")});
        Time.Nodes.Add({TEXT("Cosine"), TEXT("Cosine of time")});
        Categories.Add(Time);

        return Categories;
    }
};

REGISTER_MCP_COMMAND(FMcpGetAvailableMaterialNodesHandler)
