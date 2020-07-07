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
global_variable bool32 GLOBALAppStatus; // Loop status, the app closes if it equals 0
global_variable time_data GLOBALTime; // Called only in win32_timer and winmain

// Custom
#include "../shared_string.c"
#include "win32_debug_console.c"
#include "win32_timer.c"
#include "win32_opengl.c"
#include "win32_fileio.c"
#include "win32_input.c"
#include "win32_hot_reload.c"
#include "win32_audio_direct_sound.c"

/* TODO: 
 * os status abstraction
 * memory stuff
 * audio latency? 
 * fill audio buffer
 * fullscreen
 * control window size
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
        GLOBALTime.countsPerSecond = Win32PerformanceFrequencyGet();
        
        // NOTE: Set timers to application start
        GLOBALTime.startAppCycles = ProcessorClockCycles();
        GLOBALTime.startAppCount = Win32PerformanceCountGet();
        GLOBALTime.startAppMilliseconds = PerformanceCountToMilliseconds(
                                            GLOBALTime.startAppCount);
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

    win32_dll_code dllCode = {0};
    win32_audio_data audioData = {0};
    {
        audioData.samplesPerSecond = 48000;
        audioData.numberOfChannels = 2;
        audioData.bytesPerSample = sizeof(i16) * audioData.numberOfChannels;
        audioData.bufferSize = audioData.samplesPerSecond * audioData.bytesPerSample;
        audioData.audioLatency = audioData.samplesPerSecond / 15;
    }

    // NOTE: Get refresh rate
    f32 monitorRefreshRate = 60.f;
    {
        DEVMODEA deviceMode = {0};
        if(EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &deviceMode))
        {
            monitorRefreshRate = (f32)deviceMode.dmDisplayFrequency;
        }
    }
    GLOBALOs.timeData.targetMsPerFrame = 1 / monitorRefreshRate * 1000;

    // NOTE: init operating system interface, allocate memory etc.
    operating_system_interface *os = &GLOBALOs;
    {
        os->pernamentStorage.maxSize = Megabytes(64);
        os->temporaryStorage.maxSize = Megabytes(64);
        os->audioBufferSize = audioData.bufferSize;
        
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


        os->log = &ConsoleLog;
        os->logExtra = &ConsoleLogExtra;
        os->timeCurrentGet = &Win32TimeGetCurrent;
        os->OpenGLFunctionLoad = &Win32OpenGLFunctionLoad;
        os->VSyncSet = &Win32OpenGLSetVSync;

        LogSuccess("OS Functions Loaded");
    }

    // NOTE: Load the dll and call initialize function
    dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);
    dllCode.initialize(os);
    
    audioData.audioBuffer = Win32AudioInitialize(windowHandle, audioData.samplesPerSecond, 
                                                 audioData.bufferSize); 
    Win32AudioBufferZeroClear(&audioData);

    
    u64 beginFrameCycles = ProcessorClockCycles();

    GLOBALAppStatus = true;
    while(GLOBALAppStatus)
    {
        i64 beginFrame = Win32PerformanceCountGet();
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

        // NOTE: Figure out how much sound we want to request and figure out
        //       our position in the direct sound buffer
        DWORD playCursor = 0;
        DWORD writeCursor = 0;
        DWORD numberOfBytesToLock = 0;
        DWORD byteToLock = audioData.currentPositionInBuffer;
        bool32 soundIsValid = false;
        if(SUCCEEDED(audioData.audioBuffer->lpVtbl->GetCurrentPosition(audioData.audioBuffer, 
                                                             &playCursor, &writeCursor)))
        {
            i32 samplesOfLatency = (audioData.audioLatency * audioData.bytesPerSample);
            DWORD targetCursor = (playCursor + samplesOfLatency) % audioData.bufferSize;
            if(byteToLock > targetCursor)
            {
                numberOfBytesToLock = audioData.bufferSize - byteToLock;
                numberOfBytesToLock += targetCursor;
            }
            else numberOfBytesToLock = targetCursor - byteToLock;

            // NOTE: calculate next position in the audio buffer
            audioData.currentPositionInBuffer += numberOfBytesToLock;
            audioData.currentPositionInBuffer %= audioData.bufferSize;

            soundIsValid = true;
        }

        // NOTE: Update operating system status
        {
            os->requestedSamples = numberOfBytesToLock / audioData.bytesPerSample;
            os->samplesPerSecond = audioData.samplesPerSecond;
        }
        // NOTE: Call Update function from the dll, bit "and" operator here
        //       because we dont want update to override appstatus
        GLOBALAppStatus &= dllCode.update(os);




        if(soundIsValid)
        {
            Win32AudioBufferFill(&audioData, os->audioBuffer, 
                                 byteToLock, numberOfBytesToLock);
        }
        if(!audioData.isPlaying)
        {
            if(!SUCCEEDED(audioData.audioBuffer->lpVtbl->Play(audioData.audioBuffer, 0, 0, 
                                                            DSBPLAY_LOOPING))) 
            {
                LogError("AudioBuffer Play");
                assert(0);
            }
            audioData.isPlaying = !audioData.isPlaying;
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