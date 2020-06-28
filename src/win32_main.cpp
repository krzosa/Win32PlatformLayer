// CStandard Lib and Windows
#include <windows.h>
#include <stdio.h>
#include <assert.h>

// Opengl
#include <gl/GL.h>
#include "opengl/wglext.h"
#include "opengl/glext.h"

// NOTE: Console attach
static HANDLE GLOBALConsoleHandle;
static char GLOBALRandomAccessTextBuffer[1024];

#define log(text, ...) { sprintf(GLOBALRandomAccessTextBuffer, text, __VA_ARGS__); \
        WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer, CharLength(text), 0, 0); }

#include "typedefines.c"
#include "stringlib.c"
#include "win32_main.h"
#include "win32_opengl.c"
#include "win32_fileio.cpp"

static bool32 GLOBALisRunning = true;
static user_input GLOBALUserInput;

LRESULT CALLBACK 
Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    uint32_t VKCode = WParam;
    switch (Message) 
    {
        case WM_CLOSE:
        {
            log("WM_CLOSE\n");
            GLOBALisRunning = false;
            break;
        } 
        case WM_ACTIVATEAPP:
        {
            log("WM_ACTIVATEAPP\n");
            break;
        } 
        case WM_DESTROY:
        {
            log("WM_DESTROY\n");
            GLOBALisRunning = false;
            break;
        } 
        case WM_QUIT:
        {
            GLOBALisRunning = false;
            break;
        }

        case WM_KEYDOWN:
        {
            if(VKCode == 'W')
            {
                GLOBALUserInput.up = 1;
            }
            else if(VKCode == 'S')
            {
                GLOBALUserInput.down = 1;
            }
            if(VKCode == 'A')
            {
                GLOBALUserInput.left = 1;
            }
            else if(VKCode == 'D')
            {
                GLOBALUserInput.right = 1;
            }
            else if(VKCode == VK_ESCAPE)
            {
                GLOBALisRunning = 0;
            }
            else if(VKCode == VK_F1)
            {
                GLOBALUserInput.reset = 1;
            }
            break;
        }
        case WM_KEYUP:
        {
            if(VKCode == 'W')
            {
                GLOBALUserInput.up = 0;
            }
            else if(VKCode == 'S')
            {
                GLOBALUserInput.down = 0;
            }
            if(VKCode == 'A')
            {
                GLOBALUserInput.left = 0;
            }
            else if(VKCode == 'D')
            {
                GLOBALUserInput.right = 0;
            }
            else if(VKCode == VK_F1)
            {
                GLOBALUserInput.reset = 0;
            }
            break;
        }
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
        
    }

    return Result;
}

int 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, i32 ShowCode)
{
    // NOTE: Console Setup
    if(!AttachConsole(ATTACH_PARENT_PROCESS)) log("Failed to attach to console\n");

    GLOBALConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(!GLOBALConsoleHandle) log("Failed to get console handle\n");

    // NOTE: Window Setup
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
    
    if (!RegisterClassA(&windowClass)) log("FAILED to register windowClass\n");

    HWND windowHandle = CreateWindowExA(0, windowClass.lpszClassName, 
        "TITLE_PLACEHOLDER", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, 
        CW_USEDEFAULT, NULL, NULL, Instance, NULL);

    if(!windowHandle) log("FAILED to create a Window\n");

    HDC deviceContext = GetDC(windowHandle);
    HGLRC openglContext = InitOpenGL(deviceContext);

    v2 offset = {0, 0};

    // NOTE: Log OpenGL version
    log("OPENGL VERSION: ");
    log((char *)glGetString(GL_VERSION));
    log("\n");

    while(GLOBALisRunning)
    {
        MSG Message;
        while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }

        offset.y += GLOBALUserInput.up;
        offset.y -= GLOBALUserInput.down;
        offset.x += GLOBALUserInput.right;
        offset.x -= GLOBALUserInput.left;
        v4 lineColor = {0,255,255,255};

        glClearColor(0 + offset.x / 2000 - offset.y / 2000, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        
        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);
    }
    
    
    
    return(0);
}