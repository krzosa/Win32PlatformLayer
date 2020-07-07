typedef struct time_data
{
    // NOTE: count == QueryPerformanceCount
    // countsPerSecond says 
    // how many counts there are per second
    i64 countsPerSecond;
    f32 targetMsPerFrame;
    bool32 sleepIsGranular;

    // TimeStamp taken at the program start
    // Cycles as in processor clock cycles
    u64 startAppCycles;
    u64 startAppCount;
    f32 startAppMilliseconds;

    // Length of the update, without the sleep 
    u64 updateCycles;
    i64 updateCount;
    f32 updateMilliseconds;

    // Length of the update, with sleep
    u64 frameCycles;
    i64 frameCount;
    f32 frameMilliseconds;

    f32 framesPerSec;
    
} time_data;
