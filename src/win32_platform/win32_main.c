#include "../shared_custom.h"
#include "../shared_operating_system_interface.h"
#include "win32_main.h"

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

global_variable bool32 GLOBALAppStatus;
global_variable time_data GLOBALTime;
#define MATH_PI 3.14159265358979323846f

// Custom
#include "../shared_string.c"
#include "win32_debug_console.c"
#include "win32_timer.c"
#include "win32_opengl.c"
#include "win32_fileio.c"
#include "win32_input.c"
#include "win32_hot_reload.c"
#include "win32_sound_direct.c"

/* TODO: 
 * os status abstraction
 * add better input handling 
 * memory stuff
 * audio latency? 
 * fill audio buffer
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
        // NOTE: Set windows scheduler to wake up every 1 millisecond
        GLOBALTime.sleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
        GLOBALTime.performanceCounterFrequency = Win32PerformanceFrequencyGet();

        
        // NOTE: Set timers to application start
        GLOBALTime.startAppCycles = GetProcessorClockCycles();
        GLOBALTime.startAppCount = Win32PerformanceCountGet();
        GLOBALTime.startAppMilliseconds = PerformanceCountToMilliseconds(
                                            GLOBALTime.startAppCount);

        GLOBALTime.targetMsPerFrame = 8.f;
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

    // NOTE: Window context setup and opengl context setup
    HDC deviceContext = GetDC(windowHandle);
    HGLRC openglContext = Win32OpenGLInit(deviceContext);
    LogSuccess("OPENGL VERSION: %s", glGetString(GL_VERSION));

    Win32OpenGLAspectRatioUpdate(windowHandle, 16, 9);
    
    window_dimension win = Win32GetWindowDimension(windowHandle);
    LogInfo("Window dimmension %d %d", win.width, win.height);

    // NOTE: Construct paths to exe and to dll
    str *pathToExeDirectory = Win32ExecutableDirectoryPathGet();
    str *mainDLLPath = StringConcatChar(pathToExeDirectory, "\\app_code.dll");
    str *tempDLLPath = StringConcatChar(pathToExeDirectory, "\\app_code_temp.dll");
    LogInfo("Paths\n PathToExeDirectory: %s \n PathToDLL %s \n PathToTempDLL %s", 
        pathToExeDirectory, mainDLLPath, tempDLLPath);

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

    win32_audio_data audioData = {0};
    
    audioData.runningSampleIndex = 0;
    audioData.cyclesPerSecondOrHz = 261;
    audioData.samplesPerSecond = 48000;
    audioData.numberOfChannels = 2;
    audioData.bytesPerSample = sizeof(i16) * audioData.numberOfChannels;
    audioData.bufferSize = audioData.samplesPerSecond * audioData.bytesPerSample;
    audioData.wavePeriod = audioData.samplesPerSecond / audioData.cyclesPerSecondOrHz;

    audioData.audioBuffer = Win32AudioInitialize(windowHandle, audioData.samplesPerSecond, audioData.bufferSize); 
    Win32FillAudioBuffer(&audioData, 0, audioData.bufferSize);
    audioData.soundIsPlaying = 0;
    if(!audioData.soundIsPlaying)
    {
        if(!SUCCEEDED(audioData.audioBuffer->lpVtbl->Play(audioData.audioBuffer, 0, 0, DSBPLAY_LOOPING))) assert(0);
        audioData.soundIsPlaying = !audioData.soundIsPlaying;
    }
    

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

        // NOTE: Process input, keyboard and mouse
        Win32InputUpdate(&os.userInput);
        // NOTE: Process input, controller
        Win32XInputUpdate(&os.userInput);

        DWORD playCursor;
        DWORD writeCursor;
        if(SUCCEEDED(audioData.audioBuffer->lpVtbl->GetCurrentPosition(audioData.audioBuffer, 
                                                             &playCursor, &writeCursor)))
        {
            DWORD byteToLock = (audioData.runningSampleIndex * audioData.bytesPerSample) % audioData.bufferSize;
            DWORD numberOfBytesToLock = 0;
            if(byteToLock > playCursor)
            {
                numberOfBytesToLock = audioData.bufferSize - byteToLock;
                numberOfBytesToLock += playCursor;
            }
            else numberOfBytesToLock = playCursor - byteToLock;

            Win32FillAudioBuffer(&audioData, byteToLock, numberOfBytesToLock);
        }

        // NOTE: Call Update function from the dll, bit "and" operator here
        //       because we dont want update to override appstatus
        GLOBALAppStatus &= dllCode.update(&os);
        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);
        TimeEndFrameAndSleep(&GLOBALTime, &beginFrame, &beginFrameCycles);
    }
    
    return(1);
}

LRESULT CALLBACK 
Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    uint32_t VKCode = wParam;
    switch (message) 
    {
        case WM_QUIT:
        case WM_CLOSE:
        case WM_DESTROY:
        {
            GLOBALAppStatus = false;
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