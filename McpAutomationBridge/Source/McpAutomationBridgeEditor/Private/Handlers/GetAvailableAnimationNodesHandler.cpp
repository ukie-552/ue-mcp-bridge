#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_Base.h"

struct FAnimNodeInfo
{
    FString Name;
    FString Description;
};

struct FAnimNodeCategory
{
    FString CategoryName;
    TArray<FAnimNodeInfo> Nodes;
};

class FMcpGetAvailableAnimationNodesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_available_animation_nodes"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Category;
        Params->TryGetStringField(TEXT("category"), Category);

        FString SearchTerm;
        Params->TryGetStringField(TEXT("search"), SearchTerm);

        TArray<FAnimNodeCategory> NodeCategories = BuildNodeCategories();

        TArray<TSharedPtr<FJsonValue>> NodesArray;

        for (const FAnimNodeCategory& Cat : NodeCategories)
        {
            if (Category.IsEmpty() || Category == Cat.CategoryName)
            {
                for (const FAnimNodeInfo& Node : Cat.Nodes)
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
        for (const FAnimNodeCategory& Cat : NodeCategories)
        {
            CategoriesArray.Add(MakeShareable(new FJsonValueString(Cat.CategoryName)));
        }
        Result->SetArrayField(TEXT("categories"), CategoriesArray);

        return FMcpCommandResult::Success(Result);
    }

private:
    TArray<FAnimNodeCategory> BuildNodeCategories()
    {
        TArray<FAnimNodeCategory> Categories;

        FAnimNodeCategory StateMachine;
        StateMachine.CategoryName = TEXT("State Machine");
        StateMachine.Nodes.Add({TEXT("StateMachine"), TEXT("State machine for animation states")});
        StateMachine.Nodes.Add({TEXT("State"), TEXT("Animation state")});
        StateMachine.Nodes.Add({TEXT("StateResult"), TEXT("State result node")});
        StateMachine.Nodes.Add({TEXT("Transition"), TEXT("State transition rule")});
        StateMachine.Nodes.Add({TEXT("Conduit"), TEXT("Conduit for complex transitions")});
        Categories.Add(StateMachine);

        FAnimNodeCategory AnimAssets;
        AnimAssets.CategoryName = TEXT("Animation Assets");
        AnimAssets.Nodes.Add({TEXT("SequencePlayer"), TEXT("Plays an animation sequence")});
        AnimAssets.Nodes.Add({TEXT("SequenceEvaluator"), TEXT("Evaluates animation sequence")});
        AnimAssets.Nodes.Add({TEXT("BlendSpacePlayer"), TEXT("Plays a blend space")});
        AnimAssets.Nodes.Add({TEXT("BlendSpaceEvaluator"), TEXT("Evaluates blend space")});
        AnimAssets.Nodes.Add({TEXT("AnimSequence"), TEXT("Animation sequence reference")});
        AnimAssets.Nodes.Add({TEXT("BlendSpace"), TEXT("Blend space reference")});
        AnimAssets.Nodes.Add({TEXT("AimOffset"), TEXT("Aim offset blend space")});
        Categories.Add(AnimAssets);

        FAnimNodeCategory Blending;
        Blending.CategoryName = TEXT("Blending");
        Blending.Nodes.Add({TEXT("Blend"), TEXT("Blends two animations")});
        Blending.Nodes.Add({TEXT("BlendByBool"), TEXT("Blend based on boolean")});
        Blending.Nodes.Add({TEXT("BlendByInt"), TEXT("Blend based on integer")});
        Blending.Nodes.Add({TEXT("BlendList"), TEXT("Blend from list of animations")});
        Blending.Nodes.Add({TEXT("LayeredBlend"), TEXT("Layered blend per bone")});
        Blending.Nodes.Add({TEXT("BlendPoseByBool"), TEXT("Pose blending by boolean")});
        Blending.Nodes.Add({TEXT("ApplyAdditive"), TEXT("Apply additive animation")});
        Blending.Nodes.Add({TEXT("ApplyMeshSpaceAdditive"), TEXT("Apply mesh space additive")});
        Categories.Add(Blending);

        FAnimNodeCategory IK;
        IK.CategoryName = TEXT("IK");
        IK.Nodes.Add({TEXT("TwoBoneIK"), TEXT("Two bone inverse kinematics")});
        IK.Nodes.Add({TEXT("FABRIK"), TEXT("FABRIK IK solver")});
        IK.Nodes.Add({TEXT("HandIK"), TEXT("Hand IK solver")});
        IK.Nodes.Add({TEXT("CCDIK"), TEXT("CCD IK solver")});
        IK.Nodes.Add({TEXT("FullBodyIK"), TEXT("Full body IK")});
        IK.Nodes.Add({TEXT("SplineIK"), TEXT("Spline IK")});
        Categories.Add(IK);

        FAnimNodeCategory BoneManip;
        BoneManip.CategoryName = TEXT("Bone Manipulation");
        BoneManip.Nodes.Add({TEXT("ModifyBone"), TEXT("Modify bone transform")});
        BoneManip.Nodes.Add({TEXT("CopyBone"), TEXT("Copy bone transform")});
        BoneManip.Nodes.Add({TEXT("SkeletalControl"), TEXT("Skeletal control base")});
        BoneManip.Nodes.Add({TEXT("LookAt"), TEXT("Look at control")});
        BoneManip.Nodes.Add({TEXT("TransformBone"), TEXT("Transform bone")});
        BoneManip.Nodes.Add({TEXT("SetBoneTransform"), TEXT("Set bone transform")});
        Categories.Add(BoneManip);

        FAnimNodeCategory SkeletalMesh;
        SkeletalMesh.CategoryName = TEXT("Skeletal Mesh");
        SkeletalMesh.Nodes.Add({TEXT("SkeletalMesh"), TEXT("Skeletal mesh reference")});
        SkeletalMesh.Nodes.Add({TEXT("GetSkeletalMesh"), TEXT("Get skeletal mesh")});
        SkeletalMesh.Nodes.Add({TEXT("GetSocketLocation"), TEXT("Get socket world location")});
        SkeletalMesh.Nodes.Add({TEXT("GetSocketRotation"), TEXT("Get socket rotation")});
        SkeletalMesh.Nodes.Add({TEXT("GetBoneLocation"), TEXT("Get bone location")});
        SkeletalMesh.Nodes.Add({TEXT("GetBoneRotation"), TEXT("Get bone rotation")});
        Categories.Add(SkeletalMesh);

        FAnimNodeCategory Notifies;
        Notifies.CategoryName = TEXT("Animation Notifies");
        Notifies.Nodes.Add({TEXT("PlayMontageNotify"), TEXT("Montage notify")});
        Notifies.Nodes.Add({TEXT("AnimNotify"), TEXT("Animation notify")});
        Notifies.Nodes.Add({TEXT("AnimNotifyState"), TEXT("Animation notify state")});
        Categories.Add(Notifies);

        FAnimNodeCategory Montage;
        Montage.CategoryName = TEXT("Montage");
        Montage.Nodes.Add({TEXT("PlayMontage"), TEXT("Play animation montage")});
        Montage.Nodes.Add({TEXT("StopMontage"), TEXT("Stop montage")});
        Montage.Nodes.Add({TEXT("MontagePlay"), TEXT("Montage play node")});
        Montage.Nodes.Add({TEXT("MontageStop"), TEXT("Montage stop node")});
        Montage.Nodes.Add({TEXT("MontageIsPlaying"), TEXT("Check if montage is playing")});
        Categories.Add(Montage);

        FAnimNodeCategory RootMotion;
        RootMotion.CategoryName = TEXT("Root Motion");
        RootMotion.Nodes.Add({TEXT("RootMotion"), TEXT("Root motion extraction")});
        RootMotion.Nodes.Add({TEXT("RootMotionFromMontage"), TEXT("Root motion from montage")});
        RootMotion.Nodes.Add({TEXT("ApplyRootMotion"), TEXT("Apply root motion")});
        Categories.Add(RootMotion);

        FAnimNodeCategory Sync;
        Sync.CategoryName = TEXT("Sync");
        Sync.Nodes.Add({TEXT("SyncGroup"), TEXT("Animation sync group")});
        Sync.Nodes.Add({TEXT("Sync"), TEXT("Synchronization marker")});
        Sync.Nodes.Add({TEXT("SyncMarker"), TEXT("Sync marker reference")});
        Categories.Add(Sync);

        FAnimNodeCategory Cache;
        Cache.CategoryName = TEXT("Cache");
        Cache.Nodes.Add({TEXT("CachePose"), TEXT("Cache animation pose")});
        Cache.Nodes.Add({TEXT("UseCachedPose"), TEXT("Use cached pose")});
        Cache.Nodes.Add({TEXT("SaveCachedPose"), TEXT("Save cached pose")});
        Categories.Add(Cache);

        FAnimNodeCategory Utility;
        Utility.CategoryName = TEXT("Utility");
        Utility.Nodes.Add({TEXT("ComponentSpaceToLocalSpace"), TEXT("Convert component to local space")});
        Utility.Nodes.Add({TEXT("LocalSpaceToComponentSpace"), TEXT("Convert local to component space")});
        Utility.Nodes.Add({TEXT("ConvertSpaces"), TEXT("Space conversion")});
        Utility.Nodes.Add({TEXT("RefPose"), TEXT("Reference pose")});
        Utility.Nodes.Add({TEXT("IdentityPose"), TEXT("Identity pose")});
        Utility.Nodes.Add({TEXT("GetCurrentAsset"), TEXT("Get current animation asset")});
        Utility.Nodes.Add({TEXT("GetTimeToClosestMarker"), TEXT("Time to closest marker")});
        Categories.Add(Utility);

        return Categories;
    }
};

REGISTER_MCP_COMMAND(FMcpGetAvailableAnimationNodesHandler)
