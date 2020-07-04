typedef void console_log(char *text, ...);
typedef void console_log_extra(char *prepend, char *text, ...);
typedef void *opengl_function_load(char *name);
typedef f32 time_current_get();

typedef struct memory_storage
{
    void *memory;
    u64 maxSize;
    u64 allocatedSize;
    u64 highestAllocatedSize;
} memory_storage;

typedef struct file_data
{
    void *contents;
    u32 size;
} file_data;

typedef struct operating_system_interface
{
    memory_storage pernamentStorage;
    memory_storage temporaryStorage;

    console_log *log;
    console_log_extra *logExtra;
    time_current_get *timeCurrentGet;

    opengl_function_load *OpenGLFunctionLoad;
} operating_system_interface;

