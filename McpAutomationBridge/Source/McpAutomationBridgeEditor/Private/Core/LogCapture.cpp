#include "Core/LogCapture.h"

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

    LogQueue.Enqueue(Entry);

    while (LogQueue.Count() > MaxLogHistory)
    {
        FLogEntry OldEntry;
        LogQueue.Dequeue(OldEntry);
    }

    OnLogCaptured.Broadcast(Entry);
}

void FLogCapture::GetRecentLogs(int32 MaxCount, TArray<FLogEntry>& OutLogs)
{
    FScopeLock Lock(&LogCriticalSection);

    OutLogs.Empty();

    TArray<FLogEntry> TempLogs;
    FLogEntry Entry;
    
    while (LogQueue.Dequeue(Entry))
    {
        TempLogs.Add(Entry);
    }

    for (const FLogEntry& E : TempLogs)
    {
        LogQueue.Enqueue(E);
    }

    int32 StartIndex = FMath::Max(0, TempLogs.Num() - MaxCount);
    for (int32 i = StartIndex; i < TempLogs.Num(); ++i)
    {
        OutLogs.Add(TempLogs[i]);
    }
}

void FLogCapture::ClearLogs()
{
    FScopeLock Lock(&LogCriticalSection);

    FLogEntry Entry;
    while (LogQueue.Dequeue(Entry))
    {
    }
}
