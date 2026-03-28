#include "CoreMinimal.h"
#include "McpCommand.h"
#include "Editor.h"
#include "GameFramework/Actor.h"
#include "LevelEditor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

class FMcpShowNotificationHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("show_notification"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Message;
        if (!Params->TryGetStringField(TEXT("message"), Message))
        {
            return FMcpCommandResult::Failure(TEXT("Missing 'message' parameter"), TEXT("MISSING_PARAM"));
        }

        FString Type = TEXT("info");
        Params->TryGetStringField(TEXT("type"), Type);

        float Duration = 3.0f;
        Params->TryGetNumberField(TEXT("duration"), Duration);

        FNotificationInfo Info(FText::FromString(Message));
        Info.bFireAndForget = true;
        Info.FadeOutDuration = 0.5f;
        Info.ExpireDuration = Duration;

        SNotificationItem::ECompletionState CompletionState = SNotificationItem::CS_None;
        
        if (Type == TEXT("success"))
        {
            CompletionState = SNotificationItem::CS_Success;
        }
        else if (Type == TEXT("warning"))
        {
            CompletionState = SNotificationItem::CS_Fail;
        }
        else if (Type == TEXT("error"))
        {
            CompletionState = SNotificationItem::CS_Fail;
        }

        TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
        if (NotificationItem.IsValid())
        {
            NotificationItem->SetCompletionState(CompletionState);
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField(TEXT("shown"), true);
        Result->SetStringField(TEXT("message"), Message);
        Result->SetStringField(TEXT("type"), Type);

        return FMcpCommandResult::Success(Result);
    }
};

REGISTER_MCP_COMMAND(FMcpShowNotificationHandler)
