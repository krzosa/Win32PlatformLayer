#include <windows.h>
#include <gl/GL.h>
#include "opengl_headers/wglext.h"

#include "opengl_headers/glext.h"

typedef void *OpenGLFunctionLoadType(char *name);

// NOTE: OPENGL Pointers to functions
// NOTE: expands to, for example PFNGLBUFFERDATAPROC glBufferData;
#define GLLoad(name, type) static PFNGL##type##PROC gl##name;
// NOTE: Load OpenGL function prototypes from file
#include "opengl_procedures.include"
#undef GLLoad // undefine GLLoad macro

internal void
OpenGLFunctionsLoad(OpenGLFunctionLoadType *OpenGLFunctionLoad)
{
    // NOTE: Expands to, for example gl.UseProgram = (PFNGLUSEPROGRAMPROC)Win32OpenGLFunctionLoad("glUseProgram");
    #define GLLoad(name, type) gl##name = (PFNGL##type##PROC)OpenGLFunctionLoad("gl" #name);

    // NOTE: Load main OpenGL functions using a macro
    // Expands to glUseProgram = (PFNGLUSEPROGRAMPROC)Win32OpenGLFunctionLoad("glUseProgram");
    #include "opengl_procedures.include" // include OpenGL functions to load
    #undef GLLoad // undefine GLLoad macro 
}

