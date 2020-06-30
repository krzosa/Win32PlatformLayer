inline internal i64
Win32GetPerformanceCount()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return count.QuadPart;
}

// NOTE: Frequency = the number of counts per second
inline internal i64
Win32GetPerformanceFrequency()
{
    LARGE_INTEGER performanceCounterFrequencyResult;
    QueryPerformanceFrequency(&performanceCounterFrequencyResult);
    return performanceCounterFrequencyResult.QuadPart;
}

inline internal f32 
PerformanceCountToMilliseconds(i64 count, i64 frequency)
{
    f32 result = (f32)(count * 1000.0f) / (f32)frequency;
    return result;
}

inline internal f32
PerformanceCountToSeconds(i64 count, i64 frequency)
{
    return (f32)count / (f32)GLOBALPerformanceCounterFrequency;
}
