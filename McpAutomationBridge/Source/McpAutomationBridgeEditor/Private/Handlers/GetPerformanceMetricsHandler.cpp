#include "CoreMinimal.h"
#include "McpCommand.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformFileManager.h"
#include "Stats/Stats.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"

class FMcpGetPerformanceMetricsHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_performance_metrics"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        TSharedPtr<FJsonObject> Metrics = MakeShareable(new FJsonObject);

        CollectFrameMetrics(Metrics);
        CollectMemoryMetrics(Metrics);
        CollectWorldMetrics(Metrics);
        CollectGpuMetrics(Metrics);

        return FMcpCommandResult::Success(Metrics);
    }

private:
    void CollectFrameMetrics(TSharedPtr<FJsonObject>& OutMetrics)
    {
        float DeltaTime = FApp::GetDeltaTime();
        float FPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;
        float SmoothedFPS = FApp::GetSmoothedFPS();

        TSharedPtr<FJsonObject> FrameMetrics = MakeShareable(new FJsonObject);
        FrameMetrics->SetNumberField(TEXT("delta_time_ms"), DeltaTime * 1000.0f);
        FrameMetrics->SetNumberField(TEXT("fps"), FPS);
        FrameMetrics->SetNumberField(TEXT("smoothed_fps"), SmoothedFPS);
        FrameMetrics->SetNumberField(TEXT("frame_count"), GFrameNumber);

        OutMetrics->SetObjectField(TEXT("frame"), FrameMetrics);
    }

    void CollectMemoryMetrics(TSharedPtr<FJsonObject>& OutMetrics)
    {
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

        TSharedPtr<FJsonObject> MemoryMetrics = MakeShareable(new FJsonObject);
        MemoryMetrics->SetNumberField(TEXT("total_physical_mb"), MemStats.TotalPhysical / 1024.0 / 1024.0);
        MemoryMetrics->SetNumberField(TEXT("available_physical_mb"), MemStats.AvailablePhysical / 1024.0 / 1024.0);
        MemoryMetrics->SetNumberField(TEXT("used_physical_mb"), MemStats.UsedPhysical / 1024.0 / 1024.0);
        MemoryMetrics->SetNumberField(TEXT("peak_used_physical_mb"), MemStats.PeakUsedPhysical / 1024.0 / 1024.0);
        MemoryMetrics->SetNumberField(TEXT("total_virtual_mb"), MemStats.TotalVirtual / 1024.0 / 1024.0);
        MemoryMetrics->SetNumberField(TEXT("used_virtual_mb"), MemStats.UsedVirtual / 1024.0 / 1024.0);

        FPlatformMemoryStats LocalMemStats = FPlatformMemory::GetLocalStats();
        MemoryMetrics->SetNumberField(TEXT("local_used_physical_mb"), LocalMemStats.UsedPhysical / 1024.0 / 1024.0);

        OutMetrics->SetObjectField(TEXT("memory"), MemoryMetrics);
    }

    void CollectWorldMetrics(TSharedPtr<FJsonObject>& OutMetrics)
    {
        TSharedPtr<FJsonObject> WorldMetrics = MakeShareable(new FJsonObject);

        if (GEngine)
        {
            int32 WorldCount = 0;
            int32 ActorCount = 0;
            int32 ComponentCount = 0;

            for (const FWorldContext& Context : GEngine->GetWorldContexts())
            {
                UWorld* World = Context.World();
                if (World)
                {
                    WorldCount++;
                    ActorCount += World->GetActorCount();

                    for (TActorIterator<AActor> It(World); It; ++It)
                    {
                        if (*It)
                        {
                            ComponentCount += It->GetComponents().Num();
                        }
                    }
                }
            }

            WorldMetrics->SetNumberField(TEXT("world_count"), WorldCount);
            WorldMetrics->SetNumberField(TEXT("actor_count"), ActorCount);
            WorldMetrics->SetNumberField(TEXT("component_count"), ComponentCount);
        }

        OutMetrics->SetObjectField(TEXT("world"), WorldMetrics);
    }

    void CollectGpuMetrics(TSharedPtr<FJsonObject>& OutMetrics)
    {
        TSharedPtr<FJsonObject> GpuMetrics = MakeShareable(new FJsonObject);

        GpuMetrics->SetNumberField(TEXT("rhi_draw_calls"), 0);
        GpuMetrics->SetNumberField(TEXT("rhi_triangles_drawn"), 0);

        OutMetrics->SetObjectField(TEXT("gpu"), GpuMetrics);
    }
};

REGISTER_MCP_COMMAND(FMcpGetPerformanceMetricsHandler)
