#include "Core/LogCapture.h"
#include "Misc/OutputDeviceRedirector.h"

FLogCapture& FLogCapture::Get()
{
    static FLogCapture Instance;
    return Instance;
}

FLogCapture::FLogCapture()
{
}

FLogCapture::~FLogCapture()
{
    StopCapture();
}

void FLogCapture::StartCapture()
{
    if (bIsCapturing)
    {
        return;
    }

    GLog->AddOutputDevice(this);
    bIsCapturing = true;
}

void FLogCapture::StopCapture()
{
    if (!bIsCapturing)
    {
        return;
    }

    if (GLog)
    {
        GLog->RemoveOutputDevice(this);
    }
    bIsCapturing = false;
}

void FLogCapture::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
    if (!bIsCapturing)
    {
        return;
    }

    FScopeLock Lock(&LogCriticalSection);

    FLogEntry Entry;
    Entry.Timestamp = FDateTime::Now();
    Entry.Category = Category.ToString();
    Entry.Verbosity = Verbosity;
    Entry.Message = V;

    LogArray.Add(Entry);

    while (LogArray.Num() > MaxLogHistory)
    {
        LogArray.RemoveAt(0);
    }

    OnLogCaptured.Broadcast(Entry);
}

void FLogCapture::GetRecentLogs(int32 MaxCount, TArray<FLogEntry>& OutLogs)
{
    FScopeLock Lock(&LogCriticalSection);

    OutLogs.Empty();

    int32 StartIndex = FMath::Max(0, LogArray.Num() - MaxCount);
    for (int32 i = StartIndex; i < LogArray.Num(); ++i)
    {
        OutLogs.Add(LogArray[i]);
    }
}

void FLogCapture::ClearLogs()
{
    FScopeLock Lock(&LogCriticalSection);
    LogArray.Empty();
}
