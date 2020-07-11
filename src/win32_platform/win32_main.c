#include "../shared_language_layer.h"
#include "../shared_operating_system_interface.h"

// CStandard Lib and Windows
#include <windows.h>
#include <math.h>
#include <xinput.h>
#include <intrin.h>
#include <stdio.h>
#include "win32_main.h"

// OpenGL
#include <gl/GL.h>
#include "../opengl_headers/wglext.h"
#include "../opengl_headers/glext.h"

global_variable operating_system_interface GLOBALOs; 
global_variable bool32 GLOBALSleepIsGranular;
global_variable bool32 GLOBALApplicationIsRunning; // Loop status, the app closes if it equals 0
global_variable iv2    GLOBALDrawAreaSize;
global_variable iv2    GLOBALWindowSize;
global_variable f32    GLOBALMonitorRefreshRate;
global_variable bool32 GLOBALVSyncState;
global_variable HWND   GLOBALWindow;

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720
#define DEFAULT_WINDOW_POS_X CW_USEDEFAULT
#define DEFAULT_WINDOW_POS_Y CW_USEDEFAULT

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
 * memory stuff
 * fullscreen
 * better window resize handling
*/


int 
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, i32 showCode)
{
    // NOTE: Attach to the console that invoked the app
    Win32ConsoleAttach();

    // NOTE: Init time data
    {
        // NOTE: Set windows scheduler to wake up every 1 millisecond so
        //       so that the Sleep function will work properly for our purposes
        GLOBALSleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
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
        windowClass.style          = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc    = Win32MainWindowCallback;
        windowClass.hInstance      = instance;
        windowClass.hIcon          = LoadIcon(instance, IDI_APPLICATION);
        windowClass.hCursor        = LoadCursor(0, IDC_ARROW);
        windowClass.lpszClassName  = ("PLACEHOLDER");
    }
    
    if (!RegisterClassA(&windowClass)) {LogError("Register windowClass"); return 0;}

    GLOBALWindow = CreateWindowExA(WS_EX_LAYERED, 
                                    windowClass.lpszClassName, 
                                    "TITLE_PLACEHOLDER", 
                                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                    DEFAULT_WINDOW_POS_X, 
                                    DEFAULT_WINDOW_POS_Y, 
                                    DEFAULT_WINDOW_WIDTH, 
                                    DEFAULT_WINDOW_HEIGHT, 
                                    0, 0, instance, 0);

    if(!GLOBALWindow) {LogError("Create Window"); return 0;}
    WindowSetTransparency(255);

    // NOTE: Window context setup
    HDC deviceContext = GetDC(GLOBALWindow);

    // NOTE: Setup openGL context, choose pixel format, load wgl functions
    //       function for setting vsync is loaded here
    HGLRC openglContext = Win32OpenGLInit(deviceContext);

    LogInfo("Window size %d %d", GLOBALWindowSize.width, GLOBALWindowSize.height);
    LogInfo("Window draw area size %d %d", GLOBALDrawAreaSize.width, GLOBALDrawAreaSize.height);

    // NOTE: Set the opengl viewport to match the aspect ratio
    Win32OpenGLAspectRatioUpdate(GLOBALWindow, 16, 9);
    LogSuccess("OPENGL VERSION: %s", glGetString(GL_VERSION));
    

    // NOTE: Get monitor refresh rate
    f32 GLOBALMonitorRefreshRate = 60.f;
    {
        DEVMODEA deviceMode = {0};
        if(EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &deviceMode))
        {
            GLOBALMonitorRefreshRate = (f32)deviceMode.dmDisplayFrequency;
        }
    }
    LogInfo("Monitor refresh rate: %f", GLOBALMonitorRefreshRate);

    // NOTE: INIT WASAPI and set audio latency
    win32_audio_data audioData = Win32AudioInitialize(48000);

    // NOTE: init operating system interface, allocate memory etc.
    operating_system_interface *os = &GLOBALOs;
    {
        os->pernamentStorage.maxSize = Megabytes(64);
        os->temporaryStorage.maxSize = Megabytes(64);
        os->audioBufferSize = AudioBufferSize(audioData);
        
        os->pernamentStorage.memory = VirtualAlloc(0, 
            os->pernamentStorage.maxSize + 
            os->temporaryStorage.maxSize + 
            os->audioBufferSize, 
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );

        os->temporaryStorage.memory = ((i8 *)os->pernamentStorage.memory + os->pernamentStorage.maxSize);
        os->audioBuffer = (i8 *)os->temporaryStorage.memory + os->temporaryStorage.maxSize;
        
        LogSuccess("OS Memory allocated");

        os->audioLatencyMultiplier = 4.f;
        os->samplesPerSecond = audioData.samplesPerSecond;
        os->targetMsPerFrame = (1 / GLOBALMonitorRefreshRate * 1000);

        os->Quit                                = &Quit;
        os->TimeMillisecondsGet                 = &Win32MillisecondsGet;
        os->TimeCountsGet                       = &Win32PerformanceCountGet;
        os->ProcessorCyclesGet                  = &ProcessorCyclesGet;
        os->OpenGLFunctionLoad                  = &Win32OpenGLFunctionLoad;
        os->VSyncSet                            = &Win32OpenGLSetVSync;
        os->DrawAreaSizeGet                     = &DrawAreaSizeGet;
        os->MonitorRefreshRateGet               = &MonitorRefreshRateGet;
        os->Log                                 = &ConsoleLog;
        os->LogExtra                            = &ConsoleLogExtra;
        os->VSyncStateGet                       = &VSyncStateGet;
        os->WindowSetTransparency               = &WindowSetTransparency;
        os->WindowAlwaysOnTop                   = &WindowAlwaysOnTop;
        os->WindowNotAlwaysOnTop                = &WindowNotAlwaysOnTop;

        LogSuccess("OS Functions Loaded");
    }

    win32_dll_code dllCode = {0};

    // NOTE: Construct paths to exe and to dll
    str8 *pathToExeDirectory = Win32ExecutableDirectoryPathGet();
    str8 *mainDLLPath = StringConcatChar(pathToExeDirectory, "\\app_code.dll");
    str8 *tempDLLPath = StringConcatChar(pathToExeDirectory, "\\app_code_temp.dll");
    LogInfo("Paths\n PathToExeDirectory: %s \n PathToDLL %s \n PathToTempDLL %s", 
        pathToExeDirectory, mainDLLPath, tempDLLPath);

    // NOTE: Load the dll and call initialize function
    dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);
    dllCode.initialize(os);
    
    // NOTE: Begin timming the frame
    u64 beginFrameCycles = ProcessorClockCycles();
    i64 beginFrame = Win32PerformanceCountGet();

    GLOBALApplicationIsRunning = true;
    while(GLOBALApplicationIsRunning)
    {
        Win32UpdateDLLCode(&dllCode, mainDLLPath, tempDLLPath, os);

        // NOTE: Process input, keyboard and mouse
        Win32InputUpdate(&os->userInput);
        // NOTE: Process input, controller
        Win32XInputUpdate(&os->userInput);

        // NOTE: Figure out how much sound to write and where / update the latency based on
        // potential fps changes
        u32 samplesToWrite = Win32AudioStatusUpdate(&audioData, 
                                                    MillisecondsPerFrameToFramesPerSecond(os->targetMsPerFrame),
                                                    os->audioLatencyMultiplier);

        // NOTE: Update operating system status
        {
            os->requestedSamples = samplesToWrite;
            os->samplesPerSecond = audioData.samplesPerSecond;

            // NOTE: Call Update function from the dll, bit "and" operator here
            //       because we dont want update to override appstatus
            dllCode.update(os);
        }

        if(audioData.initialized)
        {
            Win32AudioBufferFill(samplesToWrite, os->audioBuffer, &audioData);
        }

        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);
        EndFrameAndSleep(&GLOBALOs.timeData, GLOBALOs.targetMsPerFrame, 
                         &beginFrame, &beginFrameCycles);
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
            GLOBALApplicationIsRunning = false;
            break;
        } 
        case WM_WINDOWPOSCHANGING:
        case WM_SIZE:
        {
            Win32WindowSizeUpdate(window);

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

internal void
Quit()
{
    GLOBALApplicationIsRunning = !GLOBALApplicationIsRunning;
}

internal void 
Win32WindowSizeUpdate(HWND window)
{
    RECT ClientRect;
    iv2 windowDimension;

    GetWindowRect(window, &ClientRect);
    windowDimension.width = ClientRect.right - ClientRect.left;
    windowDimension.height = ClientRect.bottom - ClientRect.top;

    GLOBALWindowSize = windowDimension;

    // get size of the window, without the border
    GetClientRect(window, &ClientRect);
    windowDimension.width = ClientRect.right - ClientRect.left;
    windowDimension.height = ClientRect.bottom - ClientRect.top;

    // NOTE: Update global window width and height
    GLOBALDrawAreaSize = windowDimension;
}

internal iv2
DrawAreaSizeGet()
{
    return GLOBALDrawAreaSize;
}

internal f32
MonitorRefreshRateGet()
{
    return GLOBALMonitorRefreshRate;
}

internal bool32
VSyncStateGet()
{
    return GLOBALVSyncState;
}

internal u64
ProcessorCyclesGet()
{
    return __rdtsc();
}

internal void
WindowAlwaysOnTop()
{
    bool32 result = SetWindowPos(GLOBALWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    if(!result)
    {
        LogError("");
        return;
    }
}

internal void
WindowNotAlwaysOnTop()
{
    bool32 result = SetWindowPos(GLOBALWindow, HWND_NOTOPMOST, 
                                 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    if(!result)
    {
        LogError("");
        return;
    }
}

internal void
WindowSetTransparency(u8 level)
{
    bool32 result = SetLayeredWindowAttributes(GLOBALWindow, 0, level, LWA_ALPHA);
    if(!result)
    {
        LogError("");
        return;
    }
}

internal void
WindowSetPosition(i32 x, i32 y)
{
    bool32 result = SetWindowPos(GLOBALWindow, 0, 
                                 x, y, 500, 500, 
                                 SWP_NOSIZE | SWP_NOOWNERZORDER);
    if(!result)
    {
        LogError("");
        return;
    }
}

internal void
WindowSetSize(i32 width, i32 height)
{
    bool32 result = SetWindowPos(GLOBALWindow, 0, 
                                 0, 0, width, height, 
                                 SWP_NOMOVE | SWP_NOOWNERZORDER);
    if(!result)
    {
        LogError("");
        return;
    }
}

internal void
WindowRefresh()
{
    bool32 result = SetWindowPos(GLOBALWindow, 0, 
                                 0, 0, 0, 0, 
                                 SWP_NOMOVE | SWP_NOOWNERZORDER | 
                                 SWP_FRAMECHANGED | SWP_NOSIZE | 
                                 SWP_SHOWWINDOW);
    if(!result)
    {
        LogError("");
        return;
    }
}

internal void
WindowDrawFrame(bool32 draw)
{
    if(draw)
    {
        if(!SetWindowLongA(GLOBALWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE))
        {
            LogError("");
        }
        WindowRefresh();

    }
    else
    {
        if(!SetWindowLongA(GLOBALWindow, GWL_STYLE, WS_VISIBLE))
        {
            LogError("");
        }
        WindowRefresh();
    }
}

internal iv2
WindowSize()
{
    return GLOBALWindowSize;
}

internal iv2
WindowDrawArea()
{
    return GLOBALDrawAreaSize;
}