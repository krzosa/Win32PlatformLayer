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


    u32 runningSampleIndex = 0;
    i32 cyclesPerSecondOrHz = 261;
    i32 samplesPerSecond = 48000;
    i32 numberOfAudioChannels = 2;
    i32 bytesPerSample = sizeof(i16) * numberOfAudioChannels;
    i32 audioBufferSize = samplesPerSecond * bytesPerSample;
    i32 squareWavePeriod = samplesPerSecond / cyclesPerSecondOrHz;

    LPDIRECTSOUNDBUFFER audioBuffer = Win32AudioInitialize(windowHandle, samplesPerSecond, audioBufferSize); 
    if(!SUCCEEDED(audioBuffer->lpVtbl->Play(audioBuffer, 0, 0, DSBPLAY_LOOPING))) assert(0);

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
        if(SUCCEEDED(audioBuffer->lpVtbl->GetCurrentPosition(audioBuffer, 
                                                             &playCursor, &writeCursor)))
        {
            i32 byteToLock = runningSampleIndex * bytesPerSample % audioBufferSize;
            DWORD numberOfBytesToLock;
            if(byteToLock > playCursor)
            {
                numberOfBytesToLock = audioBufferSize - byteToLock;
                numberOfBytesToLock += playCursor;
            }
            else if(byteToLock == playCursor) numberOfBytesToLock = audioBufferSize; 
            else numberOfBytesToLock = playCursor - byteToLock;

            VOID *region1 = 0;
            VOID *region2 = 0;
            DWORD region1Size = 0;
            DWORD region2Size = 0;
            if(SUCCEEDED(audioBuffer->lpVtbl->Lock(audioBuffer, 
                                                   byteToLock, numberOfBytesToLock, 
                                                   &region1, &region1Size, 
                                                   &region2, &region2Size, 0)))
            {
                i16 *sample = (i16 *)region1;
                i32 region1SampleCount = region1Size / bytesPerSample;

                for(i32 i = 0; i != region1SampleCount; i++)
                {
                    i16 sampleValue = runningSampleIndex++ / (squareWavePeriod / 2) % 2 ? 4000 : -4000;
                    *sample++ = sampleValue;
                    *sample++ = sampleValue;
                }

                sample = (i16 *)region2;
                i32 region2SampleCount = region2Size / bytesPerSample;

                for(i32 i = 0; i != region2SampleCount; i++)
                {
                    i16 sampleValue = runningSampleIndex++ / (squareWavePeriod / 2) % 2 ? 4000 : -4000;
                    *sample++ = sampleValue;    
                    *sample++ = sampleValue;
                }

                audioBuffer->lpVtbl->Unlock(audioBuffer, 
                                            region1, region1Size, 
                                            region2, region2Size);
            }
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