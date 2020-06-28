// NOTE: expands to, for example PFNGLBUFFERDATAPROC glBufferData;
#define GLProc(name, type) PFNGL##type##PROC name;

internal void *LoadOpenGLFunction(char *name);
internal void PrintLastErrorMessage(char *text);
internal HGLRC Win32InitOpenGL(HDC deviceContext);

// OPENGL Pointers to functions
typedef struct OpenGLFunctions
{
    GLProc(UseProgram, USEPROGRAM)
    GLProc(BindBuffer, BINDBUFFER)
    GLProc(GenBuffers, GENBUFFERS)
    GLProc(BufferData, BUFFERDATA)
} OpenGLFunctions;

static OpenGLFunctions gl = {0}; 


internal void
LoadOpenGLFunctions()
{
    #undef GLProc // undefine
    // NOTE: Expands to, for example gl.UseProgram = (PFNGLUSEPROGRAMPROC)LoadOpenGLFunction("glUseProgram");
    #define GLProc(name, type) gl.##name = (PFNGL##type##PROC)LoadOpenGLFunction("gl" #name)

    // NOTE: Load main OpenGL functions
    // Expands to glUseProgram = (PFNGLUSEPROGRAMPROC)LoadOpenGLFunction("glUseProgram");
    GLProc(UseProgram, USEPROGRAM);
    GLProc(BindBuffer, BINDBUFFER);
    GLProc(BufferData, BUFFERDATA);
    GLProc(GenBuffers, GENBUFFERS);
}
