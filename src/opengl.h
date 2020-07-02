#include <windows.h>
#include <gl/GL.h>
#include "opengl_headers/wglext.h"

#include "opengl_headers/glext.h"

// NOTE: OPENGL Pointers to functions
typedef struct OpenGLFunctions
{
    // NOTE: expands to, for example PFNGLBUFFERDATAPROC glBufferData;
    #define GLLoad(name, type) PFNGL##type##PROC name;
    // NOTE: Load OpenGL function prototypes from file
    #include "opengl_procedures.include"
    #undef GLLoad // undefine GLLoad macro
} OpenGLFunctions;
