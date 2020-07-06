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

internal f32
Win32TimeGetCurrent()
{
    i64 currentCount = Win32PerformanceCountGet();
    f32 currentSeconds = PerformanceCountToSeconds(currentCount);
    return currentSeconds - GLOBALTime.startAppMilliseconds / 1000;
}

internal void
TimeEndFrameAndSleep(time_data *time, i64 *prevFrame, u64 *prevFrameCycles)
{
    //
    // NOTE: Time the frame and sleep to hit target framerate
    //

    time->updateFrameCycles = GetProcessorClockCycles() - *prevFrameCycles;
    time->updateFrameCount = Win32PerformanceCountGet() - *prevFrame;
    time->updateMilliseconds = PerformanceCountToMilliseconds(time->updateFrameCount);
    if(time->updateMilliseconds < time->targetMsPerFrame)
    {
        time->totalMsPerFrame = time->updateMilliseconds;
        while(time->totalMsPerFrame < time->targetMsPerFrame)
        {
            DWORD timeToSleep = (DWORD)(time->targetMsPerFrame - time->totalMsPerFrame);
            if(timeToSleep > 0)
            {
                Sleep(timeToSleep);
            }
            time->totalFrameCount = Win32PerformanceCountGet() - *prevFrame;
            time->totalMsPerFrame = PerformanceCountToMilliseconds(time->totalFrameCount);
        }
    }
    else
    {
        LogInfo("Missed framerate!");
    }

    time->totalFrameCycles = GetProcessorClockCycles() - *prevFrameCycles;
    time->framesPerSec = 1 / PerformanceCountToSeconds(time->totalFrameCount); 

    Log("frame = %ffps %lucycles %fms\n", time->framesPerSec, time->totalFrameCycles, time->totalMsPerFrame);
    *prevFrame = Win32PerformanceCountGet();
    *prevFrameCycles = GetProcessorClockCycles();
}