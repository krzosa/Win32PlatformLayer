// CStandard Lib and Windows
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <xinput.h>

// Opengl
#include <gl/GL.h>
#include "opengl/wglext.h"
#include "opengl/glext.h"

// Custom
#include "typedefines.c"
#include "string.c"
#include "win32_opengl.h"
#include "win32_debug_console.c"
#include "win32_main.h"
#include "win32_opengl.c"
#include "win32_fileio.c"
#include "win32_xinput.c"
#include "win32_timer.c"

static bool32 GLOBALAppStatus = true;
static user_input GLOBALUserInput;

LRESULT CALLBACK 
Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;

    uint32_t VKCode = wParam;
    switch (message) 
    {
        case WM_CLOSE:
        {
            GLOBALAppStatus = false;
            break;
        } 
        case WM_DESTROY:
        {
            GLOBALAppStatus = false;
            break;
        } 
        case WM_QUIT:
        {
            GLOBALAppStatus = false;
            break;
        }

        // NOTE: resize opengl viewport on window resize
        case WM_SIZING:
        {
            RECT ClientRect;
            // NOTE: get size of the window, without the border
            GetClientRect(window, &ClientRect);
            int width = ClientRect.right - ClientRect.left;
            int height = ClientRect.bottom - ClientRect.top;

            glViewport(0, 0, width, height);
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
                GLOBALAppStatus = 0;
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
            Result = DefWindowProc(window, message, wParam, lParam);
        } break;
        
    }

    return Result;
}

int 
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, i32 showCode)
{
    Win32ConsoleAttach();
    
    // NOTE: Window Setup
    WNDCLASSA windowClass = {};
    {
        windowClass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
        windowClass.lpfnWndProc    = Win32MainWindowCallback;
        windowClass.cbClsExtra     = 0;
        windowClass.cbWndExtra     = 0;
        windowClass.hInstance      = instance;
        windowClass.hIcon          = LoadIcon(instance, IDI_APPLICATION);
        windowClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszMenuName   = NULL;
        windowClass.lpszClassName  = ("PLACEHOLDER");
    }
    
    if (!RegisterClassA(&windowClass)) Log("FAILED to register windowClass\n");

    HWND windowHandle = CreateWindowExA(0, windowClass.lpszClassName, 
        "TITLE_PLACEHOLDER", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, 
        CW_USEDEFAULT, NULL, NULL, instance, NULL);

    if(!windowHandle) LogError("Create Window\n");

    HDC deviceContext = GetDC(windowHandle);
    HGLRC openglContext = Win32InitOpenGL(deviceContext);
    Win32Timer timer;
    Win32TimerInit(&timer);
    Win32LoadXInput();

    v2 offset = {0, 0};

    // NOTE: Log OpenGL version
    LogInfo("OPENGL VERSION: %s", (char *)glGetString(GL_VERSION));

    char *vertexShaderSource = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;"
        "void main()"
        "{"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"
        "}\0";

    char *fragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;"
        "void main(){"
            "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);}\0";

    
    // glViewport(1000, 0, 100, 100);
    u32 vertexShader = gl.CreateShader(GL_VERTEX_SHADER);
    gl.ShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    gl.CompileShader(vertexShader);

    i32 success;
    char log[512];
    gl.GetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        gl.GetShaderInfoLog(vertexShader, 512, NULL, log);
        LogError("SHADER::VERTEX::COMPILATION_FAILED %s", log);
    }


    u32 fragmentShader = gl.CreateShader(GL_FRAGMENT_SHADER);
    gl.ShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    gl.CompileShader(fragmentShader);

    gl.GetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        gl.GetShaderInfoLog(fragmentShader, 512, NULL, log);
        LogError("SHADER::FRAGMENT::COMPILATION_FAILED %s", log);
    }

    u32 shaderProgram;
    shaderProgram = gl.CreateProgram();

    gl.AttachShader(shaderProgram, vertexShader);
    gl.AttachShader(shaderProgram, fragmentShader);

    gl.LinkProgram(shaderProgram);
    gl.GetShaderiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        gl.GetProgramInfoLog(shaderProgram, 512, NULL, log);
        LogError("SHADER VERTEX LINKING %s", log);
    }

    gl.DeleteShader(vertexShader);
    gl.DeleteShader(fragmentShader);  

    u32 vertexBufferObject, vertexArrayObject;
    gl.GenVertexArrays(1, &vertexArrayObject);
    gl.GenBuffers(1, &vertexBufferObject); 

    f32 vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    }; 

    gl.BindVertexArray(vertexArrayObject);
    gl.BindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    gl.BufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    gl.VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    gl.EnableVertexAttribArray(0);

    while(GLOBALAppStatus)
    {
        Win32TimerBeginFrame(&timer);
        MSG Message;
        while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }
        Win32UpdateXInput();

        offset.y += GLOBALUserInput.up;
        offset.y -= GLOBALUserInput.down;
        offset.x += GLOBALUserInput.right;
        offset.x -= GLOBALUserInput.left;
        v4 lineColor = {0,255,255,255};

        glClearColor(0 + offset.x / 2000 - offset.y / 2000, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        gl.UseProgram(shaderProgram);
        gl.DrawArrays(GL_TRIANGLES, 0, 3);
        
        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);
        Win32TimerEndFrame(&timer, 16);
    }
    
    
    
    return(0);
}