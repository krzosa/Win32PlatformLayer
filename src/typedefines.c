#include <stdint.h>

#define local_global static
// Function internal to the obj, file 
#define internal static

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

typedef struct v4
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} v4;