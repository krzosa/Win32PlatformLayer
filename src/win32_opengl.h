// NOTE: expands to, for example PFNGLBUFFERDATAPROC glBufferData;
#define GLLoad(name, type) PFNGL##type##PROC name;

internal void *LoadOpenGLFunction(char *name);
internal void PrintLastErrorMessage(char *text);
internal HGLRC Win32InitOpenGL(HDC deviceContext);

// NOTE: OpenGL function prototypes not included in opengl header files
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);

// NOTE: OPENGL Pointers to functions
typedef struct OpenGLFunctions
{
    #include "opengl_procedures.include"
    #undef GLLoad // undefine GLLoad macro
} OpenGLFunctions;

static OpenGLFunctions gl = {0}; 


internal void
LoadOpenGLFunctions()
{
    // NOTE: Expands to, for example gl.UseProgram = (PFNGLUSEPROGRAMPROC)LoadOpenGLFunction("glUseProgram");
    #define GLLoad(name, type) gl.##name = (PFNGL##type##PROC)LoadOpenGLFunction("gl" #name);

    // NOTE: Load main OpenGL functions
    // Expands to glUseProgram = (PFNGLUSEPROGRAMPROC)LoadOpenGLFunction("glUseProgram");
    #include "opengl_procedures.include" // include OpenGL functions to load
    #undef GLLoad // undefine GLLoad macro 
}
