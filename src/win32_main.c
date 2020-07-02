#include "shared.h"
#include "win32_main.h"

// CStandard Lib and Windows
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <xinput.h>
#include <intrin.h>

// Opengl
#include <gl/GL.h>
#include "opengl_headers/wglext.h"
#include "opengl_headers/glext.h"
#include "win32_opengl.h"

static time_data GLOBALTime;
static bool32 GLOBALAppStatus; // loop 
static user_input GLOBALUserInput;
static OpenGLFunctions gl = {0}; 

// Custom
#include "string.c"
#include "win32_debug_console.c"
#include "win32_opengl.c"
#include "win32_fileio.c"
#include "win32_xinput.c"
#include "win32_timer.c"
#include "win32_callback.c"
#include "win32_hot_reload.c"
#include "win32_wasapi.c"
#include "opengl.c"

/* TODO: 
 * os status abstraction
 * add better input handling 
 * memory stuff
 * give app more stuff to work with, test opengl in dll
 * audio latency? 
 * fill audio buffer
 * assert / compile time assert
*/

int 
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, i32 showCode)
{
    // NOTE: Init time data
    {
        // NOTE: Set timers to application start
        GLOBALTime.startAppCycles = GetProcessorClockCycles();
        GLOBALTime.startAppCount = Win32PerformanceCountGet();

        // NOTE: Set windows scheduler to wake up every 1 millisecond
        GLOBALTime.sleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
        GLOBALTime.performanceCounterFrequency = Win32PerformanceFrequencyGet();

        GLOBALTime.targetMsPerFrame = 16.6f;
    }

    // NOTE: Load XInput(xbox controllers) dynamically 
    Win32XInputLoad();
    
    // NOTE: Attach to the console that invoked the app
    Win32ConsoleAttach();

    // NOTE: Wasapi 
    audio_data audioData = Win32AudioInitialize();   
    
    // NOTE: Window Setup
    WNDCLASSA windowClass = {0};
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
    
    if (!RegisterClassA(&windowClass)) LogError("Register windowClass");

    HWND windowHandle = CreateWindowExA(0, windowClass.lpszClassName, 
        "TITLE_PLACEHOLDER", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, 
        CW_USEDEFAULT, NULL, NULL, instance, NULL);

    if(!windowHandle) LogError("Create Window");

    // NOTE: Window context setup and opengl context setup
    HDC deviceContext = GetDC(windowHandle);
    HGLRC openglContext = Win32OpenGLInit(deviceContext);
    LogSuccess("OPENGL VERSION: %s", glGetString(GL_VERSION));

    Win32OpenGLAspectRatioUpdate(windowHandle, 16, 9);


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

    u32 shaders[2];
    u32 shaderCount = 0;

    shaders[shaderCount++] = ShaderCreate(GL_VERTEX_SHADER, &vertexShaderSource);
    shaders[shaderCount++] = ShaderCreate(GL_FRAGMENT_SHADER, &fragmentShaderSource);

    u32 shaderProgram = ProgramCreate(shaders, shaderCount);

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
    gl.UseProgram(shaderProgram);

    str *pathToExeDirectory = Win32ExecutableDirectoryPathGet();
    str *mainDLLPath = StringConcatChar(pathToExeDirectory, "\\app_code.dll");
    str *tempDLLPath = StringConcatChar(pathToExeDirectory, "\\app_code_temp.dll");
    LogInfo("Paths\n PathToExeDirectory: %s \n PathToDLL %s \n PathToTempDLL %s", 
        pathToExeDirectory, mainDLLPath, tempDLLPath);

    Win32DLLCode dllCode = {0};
    application_memory memory = {0};
    dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);
    dllCode.initialize(&memory);
    
    i64 beginFrame = Win32PerformanceCountGet();
    u64 beginFrameCycles = GetProcessorClockCycles();

    GLOBALAppStatus = true;
    while(GLOBALAppStatus)
    {
        FILETIME newDLLWriteTime = Win32LastWriteTimeGet(mainDLLPath);
        if(CompareFileTime(&newDLLWriteTime, &dllCode.lastDllWriteTime) != 0)
        {
            Win32DLLCodeUnload(&dllCode);
            dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);
            dllCode.hotReload(&memory);
        }

        MSG Message;
        while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }
        Win32UpdateXInput();

        glClearColor(0, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        gl.DrawArrays(GL_TRIANGLES, 0, 3);
        dllCode.update(&memory);
        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);

        TimeEndFrameAndSleep(&GLOBALTime, &beginFrame, &beginFrameCycles);
        
    }
    
    return(1);
}