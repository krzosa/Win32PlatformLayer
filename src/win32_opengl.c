#define GLProc(name, type) PFNGL##type##PROC gl##name = 0;
#define GLLoad(name, type) gl##name = (PFNGL##type##PROC)LoadOpenGLFunction("gl" #name)

// OPENGL Pointers to functions
// expands to PFNGLBUFFERDATAPROC glBufferData = 0;
GLProc(UseProgram, USEPROGRAM)
GLProc(BindBuffer, BINDBUFFER)
GLProc(GenBuffers, GENBUFFERS)
GLProc(BufferData, BUFFERDATA)

internal void * 
LoadOpenGLFunction(char *name)
{
  void *p = (void *)wglGetProcAddress(name);
  if(p == 0 ||
    (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
    (p == (void*)-1) )
  {
    HMODULE module = LoadLibraryA("opengl32.dll");
    p = (void *)GetProcAddress(module, name);
  }

  assert(p != 0);

  return p;
}

internal void
PrintLastErrorMessage(char *text)
{
    DWORD dLastError = GetLastError();
    LPSTR strErrorMessage = NULL;
    
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | 
        FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        dLastError,
        0,
        strErrorMessage,
        0,
        NULL);

    Log("%s: %s\n", text, strErrorMessage);
}

internal HGLRC
Win32InitOpenGL(HDC deviceContext)
{
    HGLRC mainOpenglContext;
    PIXELFORMATDESCRIPTOR pixelFormat =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    i32 pixelFormatIndex = ChoosePixelFormat(deviceContext, &pixelFormat);

    if(!pixelFormatIndex)
        Log("FAILED to choose pixel format\n");
    
    if(!SetPixelFormat(deviceContext, pixelFormatIndex, &pixelFormat))
        PrintLastErrorMessage("FAILED: to set PixelFormat");

    HGLRC dummyOpenglContext = wglCreateContext(deviceContext);

    if(!dummyOpenglContext)
        PrintLastErrorMessage("FAILED: to create dummy opengl context");
    if(!wglMakeCurrent(deviceContext, dummyOpenglContext))
        PrintLastErrorMessage("FAILED: to make dummy opengl context current");

    // NOTE: Pointers to windows opengl functions
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
    PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB = 0;
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 0;

    // NOTE: Load windows opengl functions
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)LoadOpenGLFunction("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)LoadOpenGLFunction("wglCreateContextAttribsARB");
    wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)LoadOpenGLFunction("wglMakeContextCurrentARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)LoadOpenGLFunction("wglSwapIntervalEXT");

    // NOTE: Load main OpenGL functions
    // Expands to glUseProgram = (PFNGLUSEPROGRAMPROC)LoadOpenGLFunction("glUseProgram");
    GLLoad(UseProgram, USEPROGRAM);
    GLLoad(BindBuffer, BINDBUFFER);
    GLLoad(BufferData, BUFFERDATA);
    GLLoad(GenBuffers, GENBUFFERS);

    int attribList[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0
    };

    UINT numFormats = 0;
    wglChoosePixelFormatARB(deviceContext, attribList, 
        0, 1, &pixelFormatIndex, &numFormats);
        
    if(pixelFormatIndex)
    {
        const int contextAttribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            0
        };
        
        mainOpenglContext = wglCreateContextAttribsARB(deviceContext, dummyOpenglContext, contextAttribs);
        if(mainOpenglContext)
        {
            wglMakeCurrent(deviceContext, 0);
            wglDeleteContext(dummyOpenglContext);
            wglMakeCurrent(deviceContext, mainOpenglContext);
            wglSwapIntervalEXT(0);
        }
    }

    return mainOpenglContext;
    
}