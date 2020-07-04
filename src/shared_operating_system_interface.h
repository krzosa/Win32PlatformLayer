typedef void console_log(char *text, ...);
typedef void console_log_extra(char *prepend, char *text, ...);
typedef void *opengl_function_load(char *name);
typedef f32 time_current_get();

typedef struct pernament_storage
{
    void *memory;
    u64 maxSize;
    u64 allocatedSize;
} pernament_storage;

typedef struct temporary_storage
{
    void *memory;
    u64 maxSize;
    u64 allocatedSize;
    u64 highestAllocatedSize;
} temporary_storage;

typedef struct file_data
{
    void *contents;
    u32 size;
} file_data;

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

typedef struct operating_system_interface
{
    pernament_storage pernamentStorage;
    temporary_storage temporaryStorage;

    console_log *log;
    console_log_extra *logExtra;
    time_current_get *timeCurrentGet;

    opengl_function_load *OpenGLFunctionLoad;
} operating_system_interface;

