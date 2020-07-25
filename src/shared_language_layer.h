#include <stdint.h>

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

#define global_variable static
#define internal static // Function internal to the obj, file 

#define true 1
#define false 0

#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)

#define Log(text, ...)         ConsoleLog(text, __VA_ARGS__)
#define LogInfo(text, ...)     ConsoleLogExtra("INFO:    ", text, __VA_ARGS__)
#define LogSuccess(text, ...)  ConsoleLogExtra("SUCCESS: ", text, __VA_ARGS__)
#define LogError(text, ...)    ConsoleLogExtra("ERROR %s %s %d: ", text, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define Assert(expression)     if(!(expression)) PrivateSetDebuggerBreakpoint("Assert") 
#define Error(text)            PrivateSetDebuggerBreakpoint(text)
#define dbg()                  PrivateSetDebuggerBreakpoint("BREAKPOINT") 

typedef union v2
{
    struct
    {
        f32 x;
        f32 y;
    };
    struct
    {
        f32 width;
        f32 height;
    };
} v2;

typedef union iv2
{
    struct
    {
        i32 x;
        i32 y;
    };
    struct
    {
        i32 width;
        i32 height;
    };
} iv2;

#if _MSC_VER
    #define PrivateSetDebuggerBreakpoint(text) {LogError(text); __debugbreak();}
    #define SilentSetDebuggerBreakpoint() {__debugbreak();}
#else
    #define PrivateSetDebuggerBreakpoint(text) {LogError(text); *(volatile int *)0 = 0;}
    #define SilentSetDebuggerBreakpoint() { *(volatile int *)0 = 0;}
#endif