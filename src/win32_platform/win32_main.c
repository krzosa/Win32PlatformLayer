#include "../operating_system_interface.h"
#include "win32_main.h"

// CStandard Lib and Windows
#include <windows.h>
#include <stdio.h>
#include <xinput.h>
#include <intrin.h>

// OpenGL
#include <gl/GL.h>
#include "../opengl_headers/wglext.h"
#include "../opengl_headers/glext.h"

global_variable bool32 GLOBALAppStatus;
global_variable user_input GLOBALUserInput;
global_variable time_data GLOBALTime;

// Custom
#include "../string.c"
#include "win32_debug_console.c"
#include "win32_timer.c"
#include "win32_opengl.c"
#include "win32_fileio.c"
#include "win32_xinput.c"
#include "win32_callback.c"
#include "win32_hot_reload.c"
#include "win32_wasapi.c"

/* TODO: 
 * os status abstraction
 * add better input handling 
 * memory stuff
 * audio latency? 
 * fill audio buffer
*/

int 
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, i32 showCode)
{
    // NOTE: Attach to the console that invoked the app
    Win32ConsoleAttach();

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

    // NOTE: Wasapi init and load COM library dynamically 
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
    
    if (!RegisterClassA(&windowClass)) {LogError("Register windowClass"); return 0;}

    HWND windowHandle = CreateWindowExA(0, windowClass.lpszClassName, 
        "TITLE_PLACEHOLDER", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, 
        CW_USEDEFAULT, NULL, NULL, instance, NULL);

    if(!windowHandle) {LogError("Create Window"); return 0;}

    // NOTE: Window context setup and opengl context setup
    HDC deviceContext = GetDC(windowHandle);
    HGLRC openglContext = Win32OpenGLInit(deviceContext);
    LogSuccess("OPENGL VERSION: %s", glGetString(GL_VERSION));

    Win32OpenGLAspectRatioUpdate(windowHandle, 16, 9);

    // NOTE: Construct paths to exe and to dll
    str *pathToExeDirectory = Win32ExecutableDirectoryPathGet();
    str *mainDLLPath = StringConcatChar(pathToExeDirectory, "\\app_code.dll");
    str *tempDLLPath = StringConcatChar(pathToExeDirectory, "\\app_code_temp.dll");
    LogInfo("Paths\n PathToExeDirectory: %s \n PathToDLL %s \n PathToTempDLL %s", 
        pathToExeDirectory, mainDLLPath, tempDLLPath);

    Win32DLLCode dllCode = {0};

    // NOTE: init operating system interface, allocate memory etc.
    operating_system_interface os = {0};
    {
        os.pernamentStorage.memory = VirtualAlloc(
            NULL, Megabytes(64), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );
        os.pernamentStorage.maxSize = Megabytes(64);
        os.pernamentStorage.allocatedSize = 0;

        os.temporaryStorage.memory = VirtualAlloc(
            NULL, Megabytes(32), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );
        os.temporaryStorage.maxSize = Megabytes(32);
        os.temporaryStorage.allocatedSize = 0;
        
        LogSuccess("OS Memory allocated");

        os.OpenGLFunctionLoad = &Win32OpenGLFunctionLoad;

        os.log = &ConsoleLog;
        os.logExtra = &ConsoleLogExtra;

        LogSuccess("OS Functions Loaded");
    }

    // NOTE: Load the dll and call initialize function
    dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);
    dllCode.initialize(&os);
    
    i64 beginFrame = Win32PerformanceCountGet();
    u64 beginFrameCycles = GetProcessorClockCycles();

    GLOBALAppStatus = true;
    while(GLOBALAppStatus)
    {
        // NOTE: Check if dll was rebuild and load it again if it did
        FILETIME newDLLWriteTime = Win32LastWriteTimeGet(mainDLLPath);
        if(CompareFileTime(&newDLLWriteTime, &dllCode.lastDllWriteTime) != 0)
        {
            Win32DLLCodeUnload(&dllCode);
            dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);

            // NOTE: Call HotReload function from the dll
            dllCode.hotReload(&os);
        }

        MSG Message;
        while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }
        Win32UpdateXInput();

        // NOTE: Call Update function from the dll
        dllCode.update(&os);
        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);
        TimeEndFrameAndSleep(&GLOBALTime, &beginFrame, &beginFrameCycles);
    }
    
    return(1);
}