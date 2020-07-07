#include "../shared_language_layer.h"
#include "../shared_operating_system_interface.h"

// CStandard Lib and Windows
#include <windows.h>
#include <math.h>
#include <xinput.h>
#include <intrin.h>
#include <stdio.h>

// OpenGL
#include <gl/GL.h>
#include "../opengl_headers/wglext.h"
#include "../opengl_headers/glext.h"

global_variable operating_system_interface GLOBALOs; 
global_variable bool32 STATUSSleepIsGranular;
global_variable bool32 STATUSOfApplication; // Loop status, the app closes if it equals 0

// Custom
#include "../shared_string.c"
#include "win32_debug_console.c"
#include "win32_timer.c"
#include "win32_opengl.c"
#include "win32_fileio.c"
#include "win32_input.c"
#include "win32_hot_reload.c"
#include "win32_audio_wasapi.c"

/* TODO: 
 * os status abstraction
 * memory stuff
 * audio latency? 
 * fullscreen
 * control window size
 * wasapi ? ?
*/


LRESULT CALLBACK 
Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

int 
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, i32 showCode)
{
    // NOTE: Attach to the console that invoked the app
    Win32ConsoleAttach();

    // NOTE: Init time data
    {
        // NOTE: Set windows scheduler to wake up every 1 millisecond so
        //       so that the Sleep function will work properly for our purposes
        STATUSSleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
        GLOBALOs.timeData.countsPerSecond = Win32PerformanceFrequencyGet();
        
        // NOTE: Set timers to application start
        GLOBALOs.timeData.startAppCycles = ProcessorClockCycles();
        GLOBALOs.timeData.startAppCount = Win32PerformanceCountGet();
        GLOBALOs.timeData.startAppMilliseconds = PerformanceCountToMilliseconds(
                                            GLOBALOs.timeData.startAppCount);
    }

    // NOTE: Load XInput(xbox controllers) dynamically 
    Win32XInputLoad();

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

    // NOTE: Window context setup
    HDC deviceContext = GetDC(windowHandle);

    // NOTE: Setup openGL context, choose pixel format, load wgl functions
    //       function for setting vsync is loaded here
    HGLRC openglContext = Win32OpenGLInit(deviceContext);

    // NOTE: Set the opengl viewport to match the aspect ratio
    Win32OpenGLAspectRatioUpdate(windowHandle, 16, 9);
    LogSuccess("OPENGL VERSION: %s", glGetString(GL_VERSION));
    
    iv2 win = Win32GetWindowDimension(windowHandle);
    LogInfo("Window dimmension %d %d", win.width, win.height);


    // NOTE: Construct paths to exe and to dll
    str8 *pathToExeDirectory = Win32ExecutableDirectoryPathGet();
    str8 *mainDLLPath = StringConcatChar(pathToExeDirectory, "\\app_code.dll");
    str8 *tempDLLPath = StringConcatChar(pathToExeDirectory, "\\app_code_temp.dll");
    LogInfo("Paths\n PathToExeDirectory: %s \n PathToDLL %s \n PathToTempDLL %s", 
        pathToExeDirectory, mainDLLPath, tempDLLPath);

    // NOTE: Get monitor refresh rate
    f32 monitorRefreshRate = 60.f;
    {
        DEVMODEA deviceMode = {0};
        if(EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &deviceMode))
        {
            monitorRefreshRate = (f32)deviceMode.dmDisplayFrequency;
        }
    }

    win32_audio_data audioData = Win32AudioInitialize(48000);
    if(!audioData.initialized) Win32WasapiCleanup(&audioData);

    // NOTE: init operating system interface, allocate memory etc.
    operating_system_interface *os = &GLOBALOs;
    {
        os->pernamentStorage.maxSize = Megabytes(64);
        os->temporaryStorage.maxSize = Megabytes(64);
        os->audioBufferSize = AudioBufferSize(audioData);
        
        os->pernamentStorage.memory = VirtualAlloc(NULL, 
            os->pernamentStorage.maxSize + 
            os->temporaryStorage.maxSize + 
            os->audioBufferSize, 
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );

        os->temporaryStorage.memory = ((i8 *)os->pernamentStorage.memory + os->pernamentStorage.maxSize);
        os->audioBuffer = (i8 *)os->temporaryStorage.memory + os->temporaryStorage.maxSize;
        
        LogSuccess("OS Memory allocated");

        os->samplesPerSecond = audioData.samplesPerSecond;
        os->monitorRefreshRate = monitorRefreshRate;
        os->timeData.targetMsPerFrame = (1 / monitorRefreshRate * 1000);


        os->Log = &ConsoleLog;
        os->LogExtra = &ConsoleLogExtra;
        os->TimeCurrent = &Win32TimeGetCurrent;
        os->OpenGLFunctionLoad = &Win32OpenGLFunctionLoad;
        os->VSyncSet = &Win32OpenGLSetVSync;

        LogSuccess("OS Functions Loaded");
    }

    win32_dll_code dllCode = {0};

    // NOTE: Load the dll and call initialize function
    dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);
    dllCode.initialize(os);
    
    u64 beginFrameCycles = ProcessorClockCycles();
    i64 beginFrame = Win32PerformanceCountGet();

    STATUSOfApplication = true;
    while(STATUSOfApplication)
    {
        // NOTE: Check if dll was rebuild and load it again if it did
        FILETIME newDLLWriteTime = Win32LastWriteTimeGet(mainDLLPath);
        if(CompareFileTime(&newDLLWriteTime, &dllCode.lastDllWriteTime) != 0)
        {
            Win32DLLCodeUnload(&dllCode);
            dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);

            // NOTE: Call HotReload function from the dll
            dllCode.hotReload(os);
        }

        // NOTE: Process input, keyboard and mouse
        Win32InputUpdate(&os->userInput);
        // NOTE: Process input, controller
        Win32XInputUpdate(&os->userInput);

        // NOTE: Figure out how much sound to write and where 
        u32 samplesToWrite = 0;
        if(audioData.initialized)
        {
            UINT32 padding;
            if(SUCCEEDED(audioData.audioClient->lpVtbl->GetCurrentPadding(audioData.audioClient, &padding)))
            {
                samplesToWrite = audioData.latencyFrameCount - padding;
                if(samplesToWrite > audioData.latencyFrameCount)
                {
                    samplesToWrite = audioData.latencyFrameCount;
                }
            }
        }

        // NOTE: Update operating system status
        {
            os->requestedSamples = samplesToWrite;
            os->samplesPerSecond = audioData.samplesPerSecond;
        }
        // NOTE: Call Update function from the dll, bit "and" operator here
        //       because we dont want update to override appstatus
        STATUSOfApplication &= dllCode.update(os);

        if(audioData.initialized)
        {
            Win32FillAudioBuffer(samplesToWrite, os->audioBuffer, &audioData);
        }

        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);
        TimeEndFrameAndSleep(&GLOBALOs.timeData, &beginFrame, &beginFrameCycles);
    }
    
    return(1);
}

LRESULT CALLBACK 
Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    WPARAM VKCode = wParam;
    switch (message) 
    {
        case WM_QUIT:
        case WM_CLOSE:
        case WM_DESTROY:
        {
            STATUSOfApplication = false;
            break;
        } 
        case WM_WINDOWPOSCHANGING:
        case WM_SIZE:
        {
            // NOTE: resize opengl viewport on window resize
            Win32OpenGLAspectRatioUpdate(window, 16, 9);
            break;
        }
        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            assert(0);
        }
        default:
        {
            result = DefWindowProc(window, message, wParam, lParam);
        } break;

    }

    return result;
}