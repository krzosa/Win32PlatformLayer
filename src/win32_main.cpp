#include <windows.h>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>
#include <gl/GL.h>
#include "opengl/wglext.h"
#include "opengl/glext.h"


#include "typedefines.c"
#include "stringlib.c"
#include "win32_main.h"

Global bool GlobalRunning = true;
Global user_input GlobalUserInput;
Global win32_offscreen_buffer GlobalBackbuffer;

#include "software_render.cpp"
#include "win32_fileio.cpp"

#define Pi32 3.14159265359f

window_dimension 
Win32GetWindowDimension(HWND Window)
{
    RECT ClientRect;
    window_dimension windowDimension;
    // get size of the window, without the border
    GetClientRect(Window, &ClientRect);
    windowDimension.width = ClientRect.right - ClientRect.left;
    windowDimension.height = ClientRect.bottom - ClientRect.top;
    return windowDimension;
}

internal void 
Win32ResizeDIBSection(win32_offscreen_buffer* buffer, i32 width, i32 height)
{
    // if we dont free before allocating, memory will leak
    if(buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    i32 bytesPerPixel = 4;
    buffer->width = width;
    buffer->height = height;
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB; //uncompressed RGB

    
    i32 bitmapMemorySize = bytesPerPixel * (buffer->width * buffer->height);
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = width * bytesPerPixel;
}

internal void 
Win32DrawBufferToScreen(HDC DeviceContext, i32 windowWidth, i32 windowHeight, win32_offscreen_buffer* buffer)
{
    // The StretchDIBits function copies the color data for a rectangle of pixels in a DIB, 
    // to the specified destination rectangle. 
    // If the destination rectangle is larger than the source rectangle, 
    // this function stretches the rows and columns of color data to fit the destination rectangle. 
    // If the destination rectangle is smaller than the source rectangle, 
    // this function compresses the rows and columns by using the specified raster operation.
    StretchDIBits(
        DeviceContext,
        0, 0, windowWidth, windowHeight,
        0, 0, buffer->width, buffer->height,
        buffer->memory,
        &buffer->info,
        DIB_RGB_COLORS, SRCCOPY);
}

internal void
PrintLastErrorMessage(void)
{
    DWORD dLastError = GetLastError();
    LPSTR strErrorMessage = NULL;
    
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        dLastError,
        0,
        strErrorMessage,
        0,
        NULL);

    //Prints debug output to the console
    OutputDebugString(strErrorMessage);
}

LRESULT CALLBACK 
Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    uint32_t VKCode = WParam;
    switch (Message) 
    {
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            GlobalRunning = false;
            break;
        } 
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
            break;
        } 
        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
            GlobalRunning = false;
            break;
        } 
        case WM_QUIT:
        {
            GlobalRunning = false;
            break;
        }

        case WM_KEYDOWN:
        {
            if(VKCode == 'W')
            {
                GlobalUserInput.up = 1;
            }
            else if(VKCode == 'S')
            {
                GlobalUserInput.down = 1;
            }
            if(VKCode == 'A')
            {
                GlobalUserInput.left = 1;
            }
            else if(VKCode == 'D')
            {
                GlobalUserInput.right = 1;
            }
            else if(VKCode == VK_ESCAPE)
            {
                GlobalRunning = 0;
            }
            else if(VKCode == VK_F1)
            {
                GlobalUserInput.reset = 1;
            }
            break;
        }
        case WM_KEYUP:
        {
            if(VKCode == 'W')
            {
                GlobalUserInput.up = 0;
            }
            else if(VKCode == 'S')
            {
                GlobalUserInput.down = 0;
            }
            if(VKCode == 'A')
            {
                GlobalUserInput.left = 0;
            }
            else if(VKCode == 'D')
            {
                GlobalUserInput.right = 0;
            }
            else if(VKCode == VK_F1)
            {
                GlobalUserInput.reset = 0;
            }
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            window_dimension dimension = Win32GetWindowDimension(Window);
            Win32DrawBufferToScreen(DeviceContext, 
                                       dimension.width, dimension.height, 
                                       &GlobalBackbuffer);
            EndPaint(Window, &Paint);
        } break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
        
    }

    return Result;
}

internal void * 
GetAnyGLFuncAddress(const char *name)
{
  void *p = (void *)wglGetProcAddress(name);
  if(p == 0 ||
    (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
    (p == (void*)-1) )
  {
    HMODULE module = LoadLibraryA("opengl32.dll");
    p = (void *)GetProcAddress(module, name);
  }

  return p;
}

int 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, i32 ShowCode)
{
    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    WNDCLASSA windowClass = {};
    {
        windowClass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
        windowClass.lpfnWndProc    = Win32MainWindowCallback;
        windowClass.cbClsExtra     = 0;
        windowClass.cbWndExtra     = 0;
        windowClass.hInstance      = Instance;
        windowClass.hIcon          = LoadIcon(Instance, IDI_APPLICATION);
        windowClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszMenuName   = NULL;
        windowClass.lpszClassName  = ("PLACEHOLDER");
    }
    
    if (!RegisterClassA(&windowClass))
    {
        OutputDebugStringA("FAILED to register windowClass\n");
    }

    HWND windowHandle = CreateWindowExA(0, windowClass.lpszClassName, 
        "TITLE_PLACEHOLDER", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, 
        CW_USEDEFAULT, NULL, NULL, Instance, NULL);

    if(!windowHandle)
    {
        OutputDebugStringA("FAILED to create a Window\n");
    }

    HDC deviceContext = GetDC(windowHandle);
    HGLRC mainOpenglContext;

    // NOTE: OPENGL
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
    {
        OutputDebugStringA("FAILED to choose pixel format\n");
    }
    
    if(!SetPixelFormat(deviceContext, pixelFormatIndex, &pixelFormat))
    {
        PrintLastErrorMessage();
    }
    HGLRC dummyOpenglContext = wglCreateContext(deviceContext);
    if(!dummyOpenglContext)
    {
        PrintLastErrorMessage();
    }
    if(!wglMakeCurrent(deviceContext, dummyOpenglContext))
    {
        PrintLastErrorMessage();
    }

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
    PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB;
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress ("wglCreateContextAttribsARB");
    wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)wglGetProcAddress ("wglMakeContextCurrentARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress ("wglSwapIntervalEXT");

    {
        int pf_attribs_i[] =
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
        wglChoosePixelFormatARB(deviceContext, pf_attribs_i, 
            0, 1, &pixelFormatIndex, &numFormats);
    }
        
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
    

    v2 offset = {0, 0};
    i32 musicIndex = 1;
    while(GlobalRunning)
    {
        MSG Message;
        while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }

        offset.y += GlobalUserInput.up;
        offset.y -= GlobalUserInput.down;
        offset.x += GlobalUserInput.right;
        offset.x -= GlobalUserInput.left;
        v4 lineColor = {0,255,255,255};

        // DrawRectangle(0, 0, 1280, 720, 0);

        // DrawGradient(offsetX, offsetY);
        // DrawLineFirst({100, 200}, {500, 600}, lineColor);
        // DrawLineSecond({200, 200}, {600, 600}, lineColor);
        // DrawLine(10, 100, 500 + offset.x, 500 + offset.y, {0,200,100});
        glClearColor(1.0, 0, 0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // window_dimension dimension = Win32GetWindowDimension(windowHandle);
        // Win32DrawBufferToScreen(deviceContext, 
        //                            dimension.width, dimension.height, 
        //                            &GlobalBackbuffer);
        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);
    }
    
    
    
    return(0);
}