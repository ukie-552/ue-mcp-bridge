#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimLayerInterface.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/UObjectHashes.h"

class FMcpCreateAnimLayerInterfaceHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("create_anim_layer_interface"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString PackagePath;
        if (!Params->TryGetStringField(TEXT("package_path"), PackagePath))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'package_path' parameter"), TEXT("MISSING_PARAM"));
        }

        FString InterfaceName;
        if (!Params->TryGetStringField(TEXT("interface_name"), InterfaceName))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'interface_name' parameter"), TEXT("MISSING_PARAM"));
        }

        TArray<FString> InputPoseNames;
        if (Params->HasField(TEXT("input_pose_names")))
        {
            TArray<TSharedPtr<FJsonValue>> InputPoses = Params->GetArrayField(TEXT("input_pose_names"));
            for (const TSharedPtr<FJsonValue>& PoseName : InputPoses)
            {
                InputPoseNames.Add(PoseName->AsString());
            }
        }

        TArray<FString> OutputPoseNames;
        if (Params->HasField(TEXT("output_pose_names")))
        {
            TArray<TSharedPtr<FJsonValue>> OutputPoses = Params->GetArrayField(TEXT("output_pose_names"));
            for (const TSharedPtr<FJsonValue>& PoseName : OutputPoses)
            {
                OutputPoseNames.Add(PoseName->AsString());
            }
        }

        FString FullPath = FString::Printf(TEXT("%s/%s"), *PackagePath, *InterfaceName);

        UPackage* TargetPackage = CreatePackage(*FullPath);
        if (!TargetPackage)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create package: %s"), *FullPath),
                TEXT("PACKAGE_ERROR")
            );
        }

        UAnimLayerInterface* NewInterface = NewObject<UAnimLayerInterface>(TargetPackage, *InterfaceName, RF_Public | RF_Standalone);
        if (!NewInterface)
        {
            return FMcpCommandResult::Failure(
                FString::Printf(TEXT("Failed to create anim layer interface: %s"), *InterfaceName),
                TEXT("INTERFACE_CREATION_FAILED")
            );
        }

        FAssetRegistryModule::AssetCreated(NewInterface);
        NewInterface->MarkPackageDirty();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("created"), true);
        Result->SetStringField(TEXT("interface_name"), InterfaceName);
        Result->SetStringField(TEXT("interface_path"), FullPath);
        Result->SetStringField(TEXT("interface_class"), NewInterface->GetClass()->GetName());

        TArray<TSharedPtr<FJsonValue>> InputPosesArray;
        for (const FString& PoseName : InputPoseNames)
        {
            InputPosesArray.Add(MakeShareable(new FJsonValueString(PoseName)));
        }
        Result->SetArrayField(TEXT("input_poses"), InputPosesArray);

        TArray<TSharedPtr<FJsonValue>> OutputPosesArray;
        for (const FString& PoseName : OutputPoseNames)
        {
            OutputPosesArray.Add(MakeShareable(new FJsonValueString(PoseName)));
        }
        Result->SetArrayField(TEXT("output_poses"), OutputPosesArray);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpCreateAnimLayerInterfaceHandler)