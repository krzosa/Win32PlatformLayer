#include <math.h>

#define PI 3.14159265358979323846f

// multiply by degrees to get radians
#define DEG2RAD (PI/180.0f)

// multiply by radians to get degrees
#define RAD2DEG (180.0f/PI)

#if defined(KRZ_MATH_STANDALONE)

// Vector 2 { x, y }
typedef union v2
{
    struct
    {
        f32 x;
        f32 y;
    };
    struct 
    {
        f32 u;
        f32 v;
    };
    struct
    {
        f32 width;
        f32 height;
    };
    struct
    {
        f32 w;
        f32 h;
    };
    
    f32 e[2];
} v2;

// Integer Vector2 
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
    struct
    {
        i32 w;
        i32 h;
    };
    struct
    {
        i32 column;
        i32 row;
    };
    struct
    {
        i32 c;
        i32 r;
    };
    
    i32 e[2];
} iv2;


#endif
// Unsigned Integer Vector2 
typedef union uv2
{
    struct
    {
        u32 x;
        u32 y;
    };
    struct
    {
        u32 width;
        u32 height;
    };
    struct
    {
        u32 w;
        u32 h;
    };
    struct
    {
        u32 column;
        u32 row;
    };
    struct
    {
        u32 c;
        u32 r;
    };
    
    u32 e[2];
} uv2;

// Vector 3 
typedef union v3
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };
    struct
    {
        f32 width;
        f32 height;
        f32 depth;
    };
    struct
    {
        v2 xy;
        f32 z_unused;
    };
    struct
    {
        f32 x_unused;
        v2 yz;
    };
    
    f32 e[3];
} v3;


// Vector 4
typedef union v4
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct
    {
        f32 x_unused;
        f32 y_unused;
        f32 width;
        f32 height;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    struct
    {
        v2 xy_unused1;
        v2 widthHeight;
    };
    struct
    {
        v2 xy_unused2;
        v2 wh;
    };
    struct
    {
        v3 xyz;
        f32 w_unused3;
    };
    struct
    {
        v2 xy_unused4;
        v2 zw;
    };
    
    f32 e[4];
} v4;

// Matrix 4x4
typedef struct m4x4
{
    // [row][column]
    f32 e[4][4];
} m4x4;

inline f32
Round(f32 value)
{
    f32 result = roundf(value);
    return result;
}

inline f32
Floor(f32 value)
{
    f32 result = floorf(value);
    return result;
}

inline i32
RoundF32ToI32(f32 val)
{
    i32 result = (i32)Round(val);
    
    return result;
}

inline iv2
RoundV2ToIv2(v2 a)
{
    iv2 result;
    
    result.x = RoundF32ToI32(a.x);
    result.y = RoundF32ToI32(a.y);
    
    return result;
}

// WARNING(KKrzosa): Rounding works differently for minus values
// because we are using a type conversion here
inline u32
RoundF32ToU32(f32 val)
{
    u32 result = (u32)(val + 0.5f);
    
    return result;
}

// WARNING(KKrzosa): Rounding works differently for minus values
// because we are using a type conversion here
inline i32
TruncateF32ToI32(f32 val)
{
    i32 result = (i32)(val);
    
    return result;
}

// WARNING(KKrzosa): Rounding works differently for minus values
// because we are using a type conversion here
inline iv2
TruncateV2ToIv2(v2 a)
{
    iv2 result;
    
    result.x = TruncateF32ToI32(a.x);
    result.y = TruncateF32ToI32(a.y);
    
    return result;
}

// WARNING(KKrzosa): Rounding works differently for minus values
// because we are using a type conversion here
inline u32
TruncateF32ToU32(f32 val)
{
    u32 result = (u32)(val);
    
    return result;
}


inline i32
FloorF32ToI32(f32 value)
{
    i32 result = (i32)Floor(value);
    return result;
}

internal bool 
CollisionCheck(v4 rec1, v4 rec2)
{
    bool result = false;
    
    if ((rec1.x < (rec2.x + rec2.width) && (rec1.x + rec1.width) > rec2.x) &&
        (rec1.y < (rec2.y + rec2.height) && (rec1.y + rec1.height) > rec2.y)) result = true;
    
    return result;
}

internal bool 
CollisionCheck(v2 point, v4 rec)
{
    bool result = false;
    
    if ((point.x >= rec.x) && (point.x <= (rec.x + rec.width)) && (point.y >= rec.y) && (point.y <= (rec.y + rec.height))) result = true;
    
    return result;
}

inline u32
ColorToARGB(v4 color)
{
    u8 alpha = RoundF32ToU32(color.a * 255);
    u8 red =   RoundF32ToU32(color.r * 255);
    u8 green = RoundF32ToU32(color.g * 255);
    u8 blue =  RoundF32ToU32(color.b * 255);
    
    u32 result = (alpha << 24 | red << 16 | green << 8 | blue << 0);
    
    return result;
}

internal i32
Clamp(i32 min, i32 val, i32 max)
{
    if(val > max) return max;
    if(val < min) return min;
    else          return val;
}

internal u32
Clamp(u32 min, u32 val, u32 max)
{
    if(val > max) return max;
    if(val < min) return min;
    else          return val;
}

internal f32
Clamp(f32 min, f32 val, f32 max)
{
    if(val > max) return max;
    if(val < min) return min;
    else          return val;
}

// #define Min(a, b) (a) > (b) ? (b) : (a)
// #define Max(a, b) (a) > (b) ? (a) : (b)
internal i32
Min(i32 a, i32 b)
{
    if(a > b) return b;
    else      return a;
}

internal i32
Max(i32 a, i32 b)
{
    if(a > b) return a;
    else      return b;
}

internal f32
Min(f32 a, f32 b)
{
    if(a > b) return b;
    else      return a;
}

internal f32
Max(f32 a, f32 b)
{
    if(a > b) return a;
    else      return b;
}

v2
operator+(v2 a, v2 b)
{
    v2 result;
    
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    
    return result;
}

v2
operator-(v2 a, v2 b)
{
    v2 result;
    
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    
    return result;
}

v2 &
operator+=(v2 &a, v2 b)
{
    a.x = a.x + b.x;
    a.y = a.y + b.y;
    
    return a;
}

v2 &
operator-=(v2 &a, v2 b)
{
    a.x = a.x - b.x;
    a.y = a.y - b.y;
    
    return a;
}

v2 &
operator*=(v2 &a, v2 b)
{
    a.x = a.x * b.x;
    a.y = a.y * b.y;
    
    return a;
}

v2 &
operator*=(v2 &a, f32 b)
{
    a.x = a.x * b;
    a.y = a.y * b;
    
    return a;
}

v2 &
operator/=(v2 &a, v2 b)
{
    a.x = a.x / b.x;
    a.y = a.y / b.y;
    
    return a;
}

// scalar

v2
operator*(v2 a, f32 b)
{
    v2 result = {a.x * b, a.y * b};
    
    return result;
}

v2
operator*(f32 b, v2 a)
{
    v2 result = {a.x * b, a.y * b};
    
    return result;
}

v2
operator/(v2 a, f32 b)
{
    v2 result = {a.x / b, a.y / b};
    
    return result;
}

inline f32 
Length(v2 a)
{
    f32 result = sqrtf( a.x * a.x + a.y * a.y );
    return result;
}

inline f32
Dot(v2 a, v2 b)
{
    f32 result = (a.x * b.x + a.y + b.y);
    return result;
}

inline void 
Print(v2 a)
{
    Log("[ %f %f %f ]\n", a.x, a.y);
}

v2
operator-(v2 a, iv2 b)
{
    v2 result;
    
    result.x = a.x - (f32)b.x;
    result.y = a.y - (f32)b.y;
    
    return result;
}

//
// Int Vector 2
//


iv2
operator+(iv2 a, iv2 b)
{
    iv2 result;
    
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    
    return result;
}

iv2
operator-(iv2 a, iv2 b)
{
    iv2 result;
    
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    
    return result;
}

iv2
operator*(iv2 a, iv2 b)
{
    iv2 result;
    
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    
    return result;
}

iv2
operator/(iv2 a, iv2 b)
{
    iv2 result;
    
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    
    return result;
}

iv2
operator*(iv2 a, i32 b)
{
    iv2 result;
    
    result.x = a.x * b;
    result.y = a.y * b;
    
    return result;
}
iv2
operator/(iv2 a, i32 b)
{
    iv2 result;
    
    result.x = a.x / b;
    result.y = a.y / b;
    
    return result;
}
iv2
operator+(iv2 a, i32 b)
{
    iv2 result;
    
    result.x = a.x + b;
    result.y = a.y + b;
    
    return result;
}
iv2
operator-(iv2 a, i32 b)
{
    iv2 result;
    
    result.x = a.x - b;
    result.y = a.y - b;
    
    return result;
}

iv2 &
operator+=(iv2 &a, iv2 b)
{
    a.x = a.x + b.x;
    a.y = a.y + b.y;
    
    return a;
}

bool32
operator!=(iv2 a, i32 b)
{
    bool32 result = true;
    
    if(a.x == b) result = false;
    if(a.y == b) result = false;
    
    
    return result;
}

bool32
operator!=(iv2 a, iv2 b)
{
    bool32 result = false;
    
    if(a.x != b.x) result = true;
    if(a.y != b.y) result = true;
    
    
    return result;
}

bool32
operator==(iv2 a, i32 b)
{
    bool32 result = true;
    
    if(a.x != b) result = false;
    if(a.y != b) result = false;
    
    
    return result;
}


//
// Vector3
//

v3
operator+(v3 a, v3 b)
{
    v3 result;
    
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    
    return result;
}

v3
operator-(v3 a, v3 b)
{
    v3 result;
    
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    
    return result;
}

v3
operator*(v3 a, v3 b)
{
    v3 result = {a.x * b.x, a.y * b.y, a.z * b.z};
    
    return result;
}

v3
operator/(v3 a, v3 b)
{
    v3 result = {a.x / b.x, a.y / b.y, a.z / b.z};
    
    return result;
}

v3
operator-(v3 result)
{
    result.x = -result.x;
    result.y = -result.y;
    result.z = -result.z;
    
    return result;
}
v2
operator-(v2 result)
{
    result.x = -result.x;
    result.y = -result.y;
    
    return result;
}
v4
operator-(v4 result)
{
    result.x = -result.x;
    result.y = -result.y;
    result.z = -result.z;
    result.w = -result.w;
    
    return result;
}

// scalar

v3
operator*(v3 a, f32 b)
{
    v3 result = {a.x * b, a.y * b, a.z * b};
    
    return result;
}

v3
operator/(v3 a, f32 b)
{
    v3 result = {a.x / b, a.y / b, a.z / b};
    
    return result;
}

inline f32
Dot(v3 a, v3 b)
{
    f32 result = (a.x * b.x + a.y + b.y + a.z * b.z);
    return result;
}

// NOTE : Cross product gives us a vector thats perpendicular to both vectors
// which is also a normal of a plane that contains those vectors
inline v3
Cross(v3 a, v3 b)
{
    v3 result = { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
    
    return result;
}

void 
Print(v3 a)
{
    Log("[ %f %f %f ]\n", a.x, a.y, a.z);
}

void 
Print(v4 a)
{
    Log("[ %f %f %f %f ]\n", a.x, a.y, a.z, a.w);
}

//
// MATRIX 4x4
//

v4
operator*(m4x4 a, v4 b)
{
    // [row][column]
    v4 result;
    
    result.x = a.e[0][0] * b.x + a.e[0][1] * b.y + a.e[0][2] * b.z + a.e[0][3] * b.w;
    result.y = a.e[1][0] * b.x + a.e[1][1] * b.y + a.e[1][2] * b.z + a.e[1][3] * b.w;
    result.z = a.e[2][0] * b.x + a.e[2][1] * b.y + a.e[2][2] * b.z + a.e[2][3] * b.w;
    result.w = a.e[3][0] * b.x + a.e[3][1] * b.y + a.e[3][2] * b.z + a.e[3][3] * b.w;
    
    return result;
}

m4x4
operator*(m4x4 a, m4x4 b)
{
    // [row][column]
    m4x4 result;
    
    for(i32 y = 0; y < 4; y++)
    {
        for(i32 x = 0; x < 4; x++)
        {
            result.e[y][x] = a.e[y][0] * b.e[0][x] + a.e[y][1] * b.e[1][x] + a.e[y][2] * b.e[2][x] + a.e[y][3] * b.e[3][x];
        }
    }
    
    return result;
}

m4x4
operator*=(m4x4 &result, m4x4 a)
{
    result = result * a;
    
    return result;
}

inline m4x4
IdentityMatrix()
{
    m4x4 result = 
    {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    
    return result;
}


inline m4x4
ScaleMatrix(f32 x, f32 y, f32 z)
{
    m4x4 result = 
    {
        x, 0.f, 0.f, 0.f,
        0.f, y, 0.f, 0.f,
        0.f, 0.f, z, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    
    return result;
}

// returns translation matrix
inline m4x4
TranslationMatrix(f32 x, f32 y, f32 z)
{
    m4x4 result = 
    {
        1.f, 0.f, 0.f, x,
        0.f, 1.f, 0.f, y,
        0.f, 0.f, 1.f, z,
        0.f, 0.f, 0.f, 1.f
    };
    
    return result;
}

// return ortho matrix
inline m4x4
OrtographicProjectionMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    // right minus left
    f32 rml = right - left;
    f32 tmb = top - bottom;
    f32 fmn = far - near;
    
    // rpl == right plus left
    f32 rpl = right + left;
    f32 tpb = top + bottom;
    f32 fpn = far + near;
    
    m4x4 result = 
    {
        2 / (rml), 0.f,       0.f,        -((rpl) / (rml)),
        0.f,       2 / (tmb), 0.f,        -((tpb) / (tmb)),
        0.f,       0.f,      -2 / (fmn),  -((fpn) / (fmn)),
        0.f,       0.f,       0.f,        1.f
    };
    
    return result;
}

// TODO:
//https://learnopengl.com/Getting-started/Transformations 
// returns rotation matrix (angle in radians)
// inline m4x4
// MatrixRotate(v3 rotationAxis, f32 angle)
// {
//     v32 r = rotationAxis;
//     f32 cosA = cosf(angle);
//     f32 sinA = sinf(angle);

//     m4x4 result = 
//     {
//         cosA + r.x*r.x * (1 - cosA),
//         r.y * r.y * (1 - cosA),
//         r.z * r.x * (1 - cosA) - r.y * sinA,
//         0.f, 
//     }

//     return result;
// }

// returns rotation matrix (angle in radians)
inline m4x4
RotationXMatrix(f32 angle)
{
    f32 sinA = sinf(angle);
    f32 cosA = cosf(angle);
    
    m4x4 result = 
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cosA, -sinA, 0.0f,
        0.0f, sinA, cosA, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    
    return result;
}

// returns rotation matrix (angle in radians)
inline m4x4
RotationYMatrix(f32 angle)
{
    f32 sinA = sinf(angle);
    f32 cosA = cosf(angle);
    
    m4x4 result = 
    {
        cosA, 0.0f, sinA, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -sinA, 0.0f, cosA, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    
    return result;
}

// returns rotation matrix (angle in radians)
inline m4x4
RotationZMatrix(f32 angle)
{
    f32 sinA = sinf(angle);
    f32 cosA = cosf(angle);
    
    m4x4 result = 
    {
        cosA, -sinA, 0.0f, 0.0f,
        sinA, cosA, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    
    return result;
}

// returns rotation matrix (angle in radians)
inline m4x4
RotationXYZMatrix(v3 angles)
{
    m4x4 result = RotationXMatrix(angles.x) * 
        RotationYMatrix(angles.y) * 
        RotationZMatrix(angles.z);
    
    return result;
}


// Translate the matrix by the values of a vector
inline m4x4
Translate(m4x4 matrix, v3 vector)
{
    m4x4 result = matrix * TranslationMatrix(vector.x, vector.y, vector.z);
    
    return result;
}

// Scale the matrix by the values of a vector
inline m4x4
Scale(m4x4 matrix, v3 vector)
{
    m4x4 result = matrix * ScaleMatrix(vector.x, vector.y, vector.z);
    
    return result;
}

void
Print(m4x4 matrix)
{
    for(i32 y = 0; y < 4; y++)
    {
        for(i32 x = 0; x < 4; x++)
        {
            Log("%f ", matrix.e[y][x]);
        }    
        Log("\n");
    }
}

inline f32 
Radians(f32 degrees)
{
    f32 result = degrees * (PI / 180.0f);
    
    return result;
}

inline f32
Degrees(f32 radians)
{
    f32 result = radians * (180.0f / PI);
    
    return result;
}

typedef struct camera2d
{
    v3 position;
    f32 zoom;
} camera2d;

inline m4x4
CameraMatrix(camera2d camera)
{
    m4x4 result = IdentityMatrix();
    
    result = Scale(result, {camera.zoom + 1, camera.zoom + 1, 1});
    result = Translate(result, -camera.position);
    
    return result;
}
