#include "shared.h" // Opengl
#include "win32_main.h"

// CStandard Lib and Windows
#include <windows.h>
#include <stdio.h>
#include <xinput.h>
#include <intrin.h>

global_variable bool32 GLOBALAppStatus;
global_variable user_input GLOBALUserInput;

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
    time_data timeData;
    {
        // NOTE: Set timers to application start
        timeData.startAppCycles = GetProcessorClockCycles();
        timeData.startAppCount = Win32PerformanceCountGet();

        // NOTE: Set windows scheduler to wake up every 1 millisecond
        timeData.sleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
        timeData.performanceCounterFrequency = Win32PerformanceFrequencyGet();

        timeData.targetMsPerFrame = 16.6f;
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

    // NOTE: Allocate memory
    operating_system_interface applicationMemory = {0};
    {
        applicationMemory.pernamentStorage.memory = VirtualAlloc(
            NULL, Megabytes(64), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );
        applicationMemory.pernamentStorage.maxSize = Megabytes(64);
        applicationMemory.pernamentStorage.allocatedSize = 0;

        applicationMemory.temporaryStorage.memory = VirtualAlloc(
            NULL, Megabytes(32), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );
        applicationMemory.temporaryStorage.maxSize = Megabytes(32);
        applicationMemory.temporaryStorage.allocatedSize = 0;
        
        LogSuccess("Memory allocated");

        applicationMemory.OpenGLFunctionLoad = &Win32OpenGLFunctionLoad;
    }

    // NOTE: Load the dll and call initialize function
    dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);
    dllCode.initialize(&applicationMemory);
    
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
            dllCode.hotReload(&applicationMemory);
        }

        MSG Message;
        while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }
        Win32UpdateXInput();

        dllCode.update(&applicationMemory);
        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);
        TimeEndFrameAndSleep(&timeData, &beginFrame, &beginFrameCycles);
    }
    
    return(1);
}