#include "typedefines.h"

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

typedef struct operating_system_interface
{
    pernament_storage pernamentStorage;
    temporary_storage temporaryStorage;

    console_log *log;
    console_log_extra *logExtra;
    opengl_function_load *OpenGLFunctionLoad;
} operating_system_interface;

#define Log(text, ...)        ConsoleLog(text, __VA_ARGS__)
#define LogInfo(text, ...)    ConsoleLogExtra("INFO: ", text, __VA_ARGS__)
#define LogSuccess(text, ...) ConsoleLogExtra("SUCCESS: ", text, __VA_ARGS__)
#define LogError(text, ...)   ConsoleLogExtra("%s %d ERROR: ", text, __FUNCTION__, __LINE__, __VA_ARGS__)