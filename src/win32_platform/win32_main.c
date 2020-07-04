#include "../shared_custom.h"
#include "../shared_operating_system_interface.h"
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
global_variable time_data GLOBALTime;

// Custom
#include "../shared_string.c"
#include "win32_debug_console.c"
#include "win32_timer.c"
#include "win32_opengl.c"
#include "win32_fileio.c"
#include "win32_input.c"
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
        // NOTE: Set windows scheduler to wake up every 1 millisecond
        GLOBALTime.sleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
        GLOBALTime.performanceCounterFrequency = Win32PerformanceFrequencyGet();

        
        // NOTE: Set timers to application start
        GLOBALTime.startAppCycles = GetProcessorClockCycles();
        GLOBALTime.startAppCount = Win32PerformanceCountGet();
        GLOBALTime.startAppMilliseconds = PerformanceCountToMilliseconds(
                                            GLOBALTime.startAppCount);

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

    user_input userInput = {0};
    win32_dll_code dllCode = {0};

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
        os.timeCurrentGet = &Win32TimeGetCurrent;

        LogSuccess("OS Functions Loaded");
    }

    // NOTE: Load the dll and call initialize function
    dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);
    dllCode.initialize(&os);
    
    i64 beginFrame = Win32PerformanceCountGet();
    u64 beginFrameCycles = GetProcessorClockCycles();
    static f32 audioIndexer = 0.01f; 

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

        // NOTE: Process input
        Win32InputUpdate(&userInput.keyboard);
        Win32XInputUpdate();

        // f32 *audioBuffer = (f32 *)os.pernamentStorage.memory;
        // for(i32 i = 0; i < 800; i++)
        // {
        //     // debugger();
        //     audioBuffer[i] = 3000 * i;
        //     // Log("%f\n", audioBuffer[i]);
        //     audioIndexer += 0.01f;
        // }
        // Win32FillSoundBuffer(800, audioBuffer, &audioData);

        if(IsKeyDown(&userInput.keyboard, KEY_W)) Log("W\n");
        if(IsKeyPressedOnce(&userInput.keyboard, KEY_S)) Log("S\n");
        if(IsKeyUnpressedOnce(&userInput.keyboard, KEY_A)) Log("A\n");
        // if(IsKeyUp(&userInput.keyboard, KEY_D)) Log("D\n");
        

        

        // NOTE: Call Update function from the dll
        dllCode.update(&os);
        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);
        TimeEndFrameAndSleep(&GLOBALTime, &beginFrame, &beginFrameCycles);
    }
    
    return(1);
}