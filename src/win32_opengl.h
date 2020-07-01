// NOTE: expands to, for example PFNGLBUFFERDATAPROC glBufferData;
#define GLLoad(name, type) PFNGL##type##PROC name;

// NOTE: forward declarations
internal void *OpenGLFunctionLoad(char *name);
internal void ErrorMessageLastPrint(char *text);
internal HGLRC Win32OpenGLInit(HDC deviceContext);

// NOTE: OpenGL function prototypes not included in opengl header files
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);

// NOTE: OPENGL Pointers to functions
typedef struct OpenGLFunctions
{
    #include "opengl_procedures.include"
    #undef GLLoad // undefine GLLoad macro
} OpenGLFunctions;


