#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Misc/OutputDevice.h"

struct FLogEntry
{
    FDateTime Timestamp;
    FString Category;
    ELogVerbosity::Type Verbosity;
    FString Message;
};

class FLogCapture : public FOutputDevice
{
public:
    static FLogCapture& Get();

    void StartCapture();
    void StopCapture();
    
    bool IsCapturing() const { return bIsCapturing; }
    
    void GetRecentLogs(int32 MaxCount, TArray<FLogEntry>& OutLogs);
    void ClearLogs();
    void SetMaxLogHistory(int32 NewMax) { MaxLogHistory = NewMax; }
    
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnLogCaptured, const FLogEntry&);
    FOnLogCaptured OnLogCaptured;

private:
    FLogCapture();
    ~FLogCapture();

    virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;

    bool bIsCapturing = false;
    TArray<FLogEntry> LogArray;
    FCriticalSection LogCriticalSection;
    int32 MaxLogHistory = 1000;
};
