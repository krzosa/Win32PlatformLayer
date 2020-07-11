// NOTE: Pointers to windows opengl functions
internal PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
internal PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
internal PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB = 0;
internal PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 0;

// NOTE: forward declarations
internal void *Win32OpenGLFunctionLoad(char *name);
internal void PrintLastErrorMessage(char *text);
internal HGLRC Win32OpenGLInit(HDC deviceContext);

internal void * 
Win32OpenGLFunctionLoad(char *name)
{
  void *p = (void *)wglGetProcAddress(name);
  assert(p);

  return p;
}

// Pass 1 to enable vsync
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

    // NOTE: Load windows opengl functions
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)Win32OpenGLFunctionLoad("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)Win32OpenGLFunctionLoad("wglCreateContextAttribsARB");
    wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)Win32OpenGLFunctionLoad("wglMakeContextCurrentARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)Win32OpenGLFunctionLoad("wglSwapIntervalEXT");

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
        }
    }

    return mainOpenglContext;
    
}

internal void
Win32OpenGLAspectRatioUpdate(i32 ratioWidth, i32 ratioHeight)
{
    iv2 win = Win32WindowDrawAreaGetSize();

    // NOTE: keep aspect ratio of 16:9
    i32 transformedWidth = win.height * ratioWidth / ratioHeight;
    i32 centerTheThing = (win.width - transformedWidth) / 2;

    glViewport(centerTheThing, 0, transformedWidth, win.height);
}

// NOTE: Vsync is an openGL extension so it can fail
//       @returns true if success 
internal bool32
Win32OpenGLSetVSync(bool32 state)
{
    bool32 result = false;
    if(wglSwapIntervalEXT)
    {
        GLOBALVSyncState = state;
        wglSwapIntervalEXT(state);
        result = true;
    }

    return result;
}
