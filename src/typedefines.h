#include <stdint.h>
#include <assert.h>

#define global_variable static
// global variable with local scope
#define local_scoped_global static
// Function internal to the obj, file 
#define internal static

#define true 1
#define false 0

#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

typedef int8_t   bool8;
typedef int16_t  bool16;
typedef int32_t  bool32;

typedef struct v2
{
    f32 x;
    f32 y;
} v2;

// NOTE: set breakpoint
#if _MSC_VER
    #define debugger() {__debugbreak();}
#else
    #define debugger() {assert(0);}
#endif

typedef void console_log(char *text, ...);
typedef void console_log_extra(char *prepend, char *text, ...);
typedef void *opengl_function_load(char *name);