#include "typedefines.h"
#include "opengl.h"

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

typedef struct application_memory
{
    pernament_storage pernamentStorage;
    temporary_storage temporaryStorage;
    OpenGLFunctionLoadType *OpenGLFunctionLoad; 
} application_memory;