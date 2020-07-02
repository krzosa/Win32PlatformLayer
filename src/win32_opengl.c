// NOTE: forward declarations
internal void *OpenGLFunctionLoad(char *name);
internal void PrintLastErrorMessage(char *text);
internal HGLRC Win32OpenGLInit(HDC deviceContext);

internal void
OpenGLFunctionsLoad()
{
    // NOTE: Expands to, for example gl.UseProgram = (PFNGLUSEPROGRAMPROC)OpenGLFunctionLoad("glUseProgram");
    #define GLLoad(name, type) gl.##name = (PFNGL##type##PROC)OpenGLFunctionLoad("gl" #name);

    // NOTE: Load main OpenGL functions using a macro
    // Expands to glUseProgram = (PFNGLUSEPROGRAMPROC)OpenGLFunctionLoad("glUseProgram");
    #include "opengl_procedures.include" // include OpenGL functions to load
    #undef GLLoad // undefine GLLoad macro 
}


internal void * 
OpenGLFunctionLoad(char *name)
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

internal HGLRC
Win32OpenGLInit(HDC deviceContext)
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
        Win32LastErrorMessagePrint("FAILED: to set PixelFormat");

    HGLRC dummyOpenglContext = wglCreateContext(deviceContext);

    if(!dummyOpenglContext)
        Win32LastErrorMessagePrint("FAILED: to create dummy opengl context");
    if(!wglMakeCurrent(deviceContext, dummyOpenglContext))
        Win32LastErrorMessagePrint("FAILED: to make dummy opengl context current");

    // NOTE: Pointers to windows opengl functions
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
    PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB = 0;
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 0;

    // NOTE: Load windows opengl functions
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)OpenGLFunctionLoad("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)OpenGLFunctionLoad("wglCreateContextAttribsARB");
    wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)OpenGLFunctionLoad("wglMakeContextCurrentARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)OpenGLFunctionLoad("wglSwapIntervalEXT");

    OpenGLFunctionsLoad();

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

internal void
Win32OpenGLAspectRatioUpdate(HWND windowHandle, i32 ratioWidth, i32 ratioHeight)
{
    RECT ClientRect;
    // NOTE: get size of the window, without the border
    GetClientRect(windowHandle, &ClientRect);
    i32 width = ClientRect.right - ClientRect.left;
    i32 height = ClientRect.bottom - ClientRect.top;
    // NOTE: keep aspect ratio of 16:9
    i32 transformedWidth = height * ratioWidth / ratioHeight;
    i32 centerTheThing = (width - transformedWidth) / 2;

    glViewport(centerTheThing, 0, transformedWidth, height);
}
