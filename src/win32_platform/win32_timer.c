#define GetProcessorClockCycles() __rdtsc()

inline internal i64
Win32PerformanceCountGet()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return count.QuadPart;
}

// NOTE: Frequency = the number of counts per second
inline internal i64
Win32PerformanceFrequencyGet()
{
    LARGE_INTEGER performanceCounterFrequencyResult;
    QueryPerformanceFrequency(&performanceCounterFrequencyResult);
    return performanceCounterFrequencyResult.QuadPart;
}

inline internal f32
PerformanceCountToMilliseconds(i64 count)
{
    f32 result = (f32)(count * 1000.0f) / (f32)GLOBALTime.performanceCounterFrequency;
    return result;
}

inline internal f32
PerformanceCountToSeconds(i64 count)
{
    f32 result = (f32)count / (f32)GLOBALTime.performanceCounterFrequency;
    return result;
}

inline internal f32
PerformanceCountToFramesPerSecond(i64 count)
{
    f32 result = 1 / ((f32)count / (f32)GLOBALTime.performanceCounterFrequency);
    return result;
}

internal void
TimeEndFrameAndSleep(time_data *time, i64 *prevFrame, u64 *prevFrameCycles)
{
    //
    // NOTE: Time the frame and sleep to hit target framerate
    //

    time->updateFrameCycles = GetProcessorClockCycles() - *prevFrameCycles;
    time->updateFrameCount = Win32PerformanceCountGet() - *prevFrame;
    f32 msPerUpdate = PerformanceCountToMilliseconds(time->updateFrameCount);
    if(msPerUpdate < time->targetMsPerFrame)
    {
        if(time->sleepIsGranular)
        {
            Sleep(time->targetMsPerFrame - msPerUpdate);
        }
        else
        {
            LogError("Sleep is not granular!");
        }
    }
    else
    {
        LogError("Missed framerate!");
    }

    time->totalFrameCount = Win32PerformanceCountGet() - *prevFrame;
    time->totalFrameCycles = GetProcessorClockCycles() - *prevFrameCycles;
    f32 totalMsPerFrame = PerformanceCountToMilliseconds(time->totalFrameCount);
    f32 framesPerSec = 1 / PerformanceCountToSeconds(time->totalFrameCount); 

    Log("frame = %ffps %lucycles %fms\n", framesPerSec, time->totalFrameCycles, totalMsPerFrame);
    *prevFrame = Win32PerformanceCountGet();
    *prevFrameCycles = GetProcessorClockCycles();
}