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

typedef void console_log(char *text, ...);
typedef void console_log_extra(char *prepend, char *text, ...);
typedef void *opengl_function_load(char *name);

typedef struct operating_system_interface
{
    pernament_storage pernamentStorage;
    temporary_storage temporaryStorage;
    opengl_function_load *OpenGLFunctionLoad;

    console_log *log;
    console_log_extra *logExtra;
} operating_system_interface;

#include "opengl.h"