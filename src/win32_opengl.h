// NOTE: expands to, for example PFNGLBUFFERDATAPROC glBufferData;
#define GLProc(name, type) PFNGL##type##PROC name;

internal void *LoadOpenGLFunction(char *name);
internal void PrintLastErrorMessage(char *text);
internal HGLRC Win32InitOpenGL(HDC deviceContext);

// NOTE: OpenGL function prototypes not included in opengl header files
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);

// NOTE: OPENGL Pointers to functions
typedef struct OpenGLFunctions
{
    GLProc(BindBuffer, BINDBUFFER)
    GLProc(GenBuffers, GENBUFFERS)
    GLProc(BufferData, BUFFERDATA)
    GLProc(BindVertexArray, BINDVERTEXARRAY)

    GLProc(CreateShader, CREATESHADER)
    GLProc(CompileShader, COMPILESHADER)
    GLProc(ShaderSource, SHADERSOURCE)
    GLProc(AttachShader, ATTACHSHADER)
    GLProc(DeleteShader, DELETESHADER)

    GLProc(CreateProgram, CREATEPROGRAM)
    GLProc(LinkProgram, LINKPROGRAM)
    GLProc(UseProgram, USEPROGRAM)

    GLProc(EnableVertexAttribArray, ENABLEVERTEXATTRIBARRAY)
    GLProc(DisableVertexAttribArray, DISABLEVERTEXATTRIBARRAY)
    GLProc(VertexAttribPointer, VERTEXATTRIBPOINTER)

    GLProc(DrawArrays, DRAWARRAYS)
    GLProc(GetShaderiv, GETSHADERIV)
    GLProc(GetProgramiv, GETPROGRAMIV)
    GLProc(GetShaderInfoLog, GETSHADERINFOLOG)
    GLProc(GetProgramInfoLog, GETPROGRAMINFOLOG)
    GLProc(GenVertexArrays, GENVERTEXARRAYS)
} OpenGLFunctions;

static OpenGLFunctions gl = {0}; 


internal void
LoadOpenGLFunctions()
{
    #undef GLProc // undefine
    // NOTE: Expands to, for example gl.UseProgram = (PFNGLUSEPROGRAMPROC)LoadOpenGLFunction("glUseProgram");
    #define GLProc(name, type) gl.##name = (PFNGL##type##PROC)LoadOpenGLFunction("gl" #name);

    // NOTE: Load main OpenGL functions
    // Expands to glUseProgram = (PFNGLUSEPROGRAMPROC)LoadOpenGLFunction("glUseProgram");
    GLProc(BindBuffer, BINDBUFFER)
    GLProc(GenBuffers, GENBUFFERS)
    GLProc(BufferData, BUFFERDATA)
    GLProc(BindVertexArray, BINDVERTEXARRAY)

    GLProc(CreateShader, CREATESHADER)
    GLProc(CompileShader, COMPILESHADER)
    GLProc(ShaderSource, SHADERSOURCE)
    GLProc(AttachShader, ATTACHSHADER)
    GLProc(DeleteShader, DELETESHADER)

    GLProc(CreateProgram, CREATEPROGRAM)
    GLProc(LinkProgram, LINKPROGRAM)
    GLProc(UseProgram, USEPROGRAM)

    GLProc(EnableVertexAttribArray, ENABLEVERTEXATTRIBARRAY)
    GLProc(DisableVertexAttribArray, DISABLEVERTEXATTRIBARRAY)
    GLProc(VertexAttribPointer, VERTEXATTRIBPOINTER)

    GLProc(DrawArrays, DRAWARRAYS)
    GLProc(GetShaderiv, GETSHADERIV)
    GLProc(GetShaderInfoLog, GETSHADERINFOLOG)
    GLProc(GetProgramiv, GETPROGRAMIV)
    GLProc(GetProgramInfoLog, GETPROGRAMINFOLOG)
    GLProc(GenVertexArrays, GENVERTEXARRAYS)
    GLProc(BindVertexArray, BINDVERTEXARRAY)
}
