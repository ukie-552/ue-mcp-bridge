#include "CoreMinimal.h"
#include "McpCommand.h"
#include "GameFramework/PlayerController.h"
#include "Editor.h"
#include "EditorViewportClient.h"

class FMcpSetViewportCameraHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("set_viewport_camera"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FVector Location = FVector::ZeroVector;
        const TSharedPtr<FJsonObject>* LocationObj;
        if (Params->TryGetObjectField(TEXT("location"), LocationObj))
        {
            double X = 0, Y = 0, Z = 0;
            (*LocationObj)->TryGetNumberField(TEXT("x"), X);
            (*LocationObj)->TryGetNumberField(TEXT("y"), Y);
            (*LocationObj)->TryGetNumberField(TEXT("z"), Z);
            Location = FVector(X, Y, Z);
        }

        FRotator Rotation = FRotator::ZeroRotator;
        const TSharedPtr<FJsonObject>* RotationObj;
        if (Params->TryGetObjectField(TEXT("rotation"), RotationObj))
        {
            double Pitch = 0, Yaw = 0, Roll = 0;
            (*RotationObj)->TryGetNumberField(TEXT("pitch"), Pitch);
            (*RotationObj)->TryGetNumberField(TEXT("yaw"), Yaw);
            (*RotationObj)->TryGetNumberField(TEXT("roll"), Roll);
            Rotation = FRotator(Pitch, Yaw, Roll);
        }

        FViewport* Viewport = GEditor->GetActiveViewport();
        if (!Viewport)
        {
            return FMcpCommandResult::Failure(TEXT("No active viewport"), TEXT("NO_VIEWPORT"));
        }

        FEditorViewportClient* ViewportClient = (FEditorViewportClient*)Viewport->GetClient();
        if (!ViewportClient)
        {
            return FMcpCommandResult::Failure(TEXT("No viewport client"), TEXT("NO_VIEWPORT_CLIENT"));
        }

        ViewportClient->SetViewLocation(Location);
        ViewportClient->SetViewRotation(Rotation);
        ViewportClient->Invalidate();

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("set"), true);

        TSharedPtr<FJsonObject> LocJson = MakeShareable(new FJsonObject);
        LocJson->SetNumberField(TEXT("x"), Location.X);
        LocJson->SetNumberField(TEXT("y"), Location.Y);
        LocJson->SetNumberField(TEXT("z"), Location.Z);
        Result->SetObjectField(TEXT("location"), LocJson);

        TSharedPtr<FJsonObject> RotJson = MakeShareable(new FJsonObject);
        RotJson->SetNumberField(TEXT("pitch"), Rotation.Pitch);
        RotJson->SetNumberField(TEXT("yaw"), Rotation.Yaw);
        RotJson->SetNumberField(TEXT("roll"), Rotation.Roll);
        Result->SetObjectField(TEXT("rotation"), RotJson);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpSetViewportCameraHandler)
