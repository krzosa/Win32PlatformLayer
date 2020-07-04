typedef struct window_dimension
{
    i32 width;
    i32 height;
} window_dimension;

typedef struct time_data
{
    // NOTE: count == QueryPerformanceCount
    // performanceCounterFrequency says 
    // how many counts there are per second
    i64 performanceCounterFrequency;

    // Can we assume that Sleep will get called every millisecond
    bool32 sleepIsGranular;
    f32 targetMsPerFrame;

    // TimeStamp taken at the program start
    // Cycles as in processor clock cycles
    u64 startAppCycles;
    u64 startAppCount;
    f32 startAppMilliseconds;

    // Length of the update, without the sleep 
    u64 updateFrameCycles;
    i64 updateFrameCount;
    f32 updateMilliseconds;

    // Length of the update, with sleep
    u64 totalFrameCycles;
    i64 totalFrameCount;
    f32 totalMsPerFrame;

    f32 framesPerSec;
    
} time_data;