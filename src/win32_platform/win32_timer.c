#define ProcessorClockCycles() __rdtsc()

inline internal i64
Win32PerformanceCountGet()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return count.QuadPart;
}

inline internal f32
Win32SecondsGet()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return ((f32)count.QuadPart / (f32)GLOBALTime.countsPerSecond);
}

inline internal f32
Win32MillisecondsGet()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return ((f32)(count.QuadPart * 1000) / (f32)GLOBALTime.countsPerSecond);
}

// NOTE: Frequency = the number of counts per second
inline internal i64
Win32PerformanceFrequencyGet()
{
    LARGE_INTEGER countsPerSecondResult;
    QueryPerformanceFrequency(&countsPerSecondResult);
    return countsPerSecondResult.QuadPart;
}

inline internal f32
PerformanceCountToMilliseconds(i64 count)
{
    f32 result = (f32)(count * 1000.0f) / (f32)GLOBALTime.countsPerSecond;
    return result;
}

inline internal f32
PerformanceCountToSeconds(i64 count)
{
    f32 result = (f32)count / (f32)GLOBALTime.countsPerSecond;
    return result;
}

inline internal f32
PerformanceCountToFramesPerSecond(i64 count)
{
    f32 result = 1 / ((f32)count / (f32)GLOBALTime.countsPerSecond);
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
TimeEndFrameAndSleep(time_data *time, i64 *prevFrame, u64 *prevFrameCycles, f32 targetMsPerFrame)
{
    //
    // NOTE: Time the frame and sleep to hit target framerate
    //
    time->updateCount = Win32PerformanceCountGet() - *prevFrame;
    time->updateCycles = ProcessorClockCycles() - *prevFrameCycles;
    time->updateMilliseconds = PerformanceCountToMilliseconds(time->updateCount);

    time->frameMilliseconds = time->updateMilliseconds;
    if(time->frameMilliseconds < targetMsPerFrame)
    {
        if(time->sleepIsGranular)
        {
            // TODO: Test on varied frame rate
            f32 timeToSleep = (targetMsPerFrame - time->frameMilliseconds) - 1.0f;
            if(timeToSleep > 0)
            {
                Sleep((DWORD)timeToSleep);
            }
        }

        // NOTE: report if slept too much
        time->frameCount = Win32PerformanceCountGet() - *prevFrame;
        time->frameMilliseconds = PerformanceCountToMilliseconds(time->frameCount);
        if(time->frameMilliseconds > targetMsPerFrame) 
        {
            LogInfo("Slept too much!");
        }

        // NOTE: stall if we didnt hit the final ms per frame
        while(time->frameMilliseconds < targetMsPerFrame)
        {
            time->frameCount = Win32PerformanceCountGet() - *prevFrame;
            time->frameMilliseconds = PerformanceCountToMilliseconds(time->frameCount);
        }
    }
    else
    {
        LogInfo("Missed framerate!");
    }

    time->frameCount = Win32PerformanceCountGet() - *prevFrame;
    time->frameMilliseconds = PerformanceCountToMilliseconds(time->frameCount);
    time->frameCycles = ProcessorClockCycles() - *prevFrameCycles;

    Log("frame = %fms %fcycles\n", time->updateMilliseconds, time->frameMilliseconds);
    *prevFrameCycles = ProcessorClockCycles();
    *prevFrame = Win32PerformanceCountGet();
}


