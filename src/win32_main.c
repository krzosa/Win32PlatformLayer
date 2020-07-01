#include "shared.h"
#include "win32_main.h"

// CStandard Lib and Windows
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <xinput.h>
#include <intrin.h>
#include <objbase.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>

// Opengl
#include <gl/GL.h>
#include "opengl_headers/wglext.h"
#include "opengl_headers/glext.h"

static time_data GLOBALTime;
static bool32 GLOBALAppStatus;
static user_input GLOBALUserInput;

// Custom
#include "string.c"
#include "win32_opengl.h"
#include "win32_debug_console.c"
#include "win32_opengl.c"
#include "win32_fileio.c"
#include "win32_xinput.c"
#include "win32_timer.c"
#include "win32_callback.c"
#include "win32_hot_reload.c"
#include "opengl.c"

/* TODO: 
 * os status abstraction
 * add wasapi audio init
 * add better input handling 
 * either implement string lib in platform or not
 * hot reload
 * memory stuff
*/

static const GUID IID_IAudioClient = {0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2};
static const GUID IID_IAudioRenderClient = {0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2};
static const GUID CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E};
static const GUID IID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6};
static const GUID PcmSubformatGuid = {STATIC_KSDATAFORMAT_SUBTYPE_PCM};

int 
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, i32 showCode)
{
    // NOTE: Init time data
    {
        // NOTE: Set timers to application start
        GLOBALTime.startAppCycles = GetProcessorClockCycles();
        GLOBALTime.startAppCount = Win32GetPerformanceCount();

        // NOTE: Set windows scheduler to wake up every 1 millisecond
        GLOBALTime.sleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
        GLOBALTime.performanceCounterFrequency = Win32GetPerformanceFrequency();

        GLOBALTime.targetMsPerFrame = 16.6f;
    }

    // NOTE: Attach to the console that invoked the app
    Win32ConsoleAttach();
    
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
    HGLRC openglContext = Win32InitOpenGL(deviceContext);
    Win32MaintainAspectRatio(windowHandle, 16, 9);
    Win32LoadXInput();

    LogInfo("OPENGL VERSION: %s", (char *)glGetString(GL_VERSION));

    // TODO:

    // CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY);

    // HRESULT result;

	// IMMDeviceEnumerator *deviceEnum = NULL;
	// result = CoCreateInstance(
	// 	&CLSID_MMDeviceEnumerator, NULL,
	// 	CLSCTX_ALL, &IID_IMMDeviceEnumerator,
	// 	(LPVOID *)&deviceEnum);

    // if(result != S_OK)
    // {
    //     LogError("CoCreateInstance");
    //     goto wasapiInitEnd;
    // }

    // IMMDevice *device = NULL;
	// result = deviceEnum->lpVtbl->GetDefaultAudioEndpoint(deviceEnum, eRender, eConsole, &device);

    // if(result != S_OK)
    // {
    //     LogError("GetDefaultAudioEndpoint");
    //     goto wasapiInitEnd;
    // }

    // IAudioClient *audioClient;
    // result = device->lpVtbl->Activate(device, &IID_IAudioClient, CLSCTX_ALL, 0, (void **)&audioClient);

    // if(result != S_OK)
    // {
    //     LogError("IAudioClient Activate");
    //     goto wasapiInitEnd;
    // }

    // wasapiInitEnd:;

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

    shaders[shaderCount++] = CreateShader(GL_VERTEX_SHADER, &vertexShaderSource);
    shaders[shaderCount++] = CreateShader(GL_FRAGMENT_SHADER, &fragmentShaderSource);

    u32 shaderProgram = CreateProgram(shaders, shaderCount);

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

    str *pathToExe = ExecutablePathGet();
    char mainDLLPath[] = "app_code.dll";
    char tempDLLPath[] = "app_code_temp.dll";

    Win32DLLCode dllCode = {0};
    AppMemory memory = {0};
    dllCode = Win32LoadDLLCode(mainDLLPath, tempDLLPath);
    dllCode.initialize(&memory);
    
    i64 beginFrame = Win32GetPerformanceCount();
    u64 beginFrameCycles = GetProcessorClockCycles();

    GLOBALAppStatus = true;
    while(GLOBALAppStatus)
    {
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
        
        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);

        //
        // NOTE: Time the frame and sleep to hit target framerate
        //
        {
            GLOBALTime.updateFrameCycles = GetProcessorClockCycles() - beginFrameCycles;
            GLOBALTime.updateFrameCount = Win32GetPerformanceCount() - beginFrame;

            f32 msPerUpdate = PerformanceCountToMilliseconds(GLOBALTime.updateFrameCount);
            if(msPerUpdate < GLOBALTime.targetMsPerFrame)
            {
                if(GLOBALTime.sleepIsGranular)
                {
                    Sleep(GLOBALTime.targetMsPerFrame - msPerUpdate);
                }
                else
                {
                    LogError("Sleep is not granular!");
                }
            }
            else
            {
                LogError("Missed framerate!");
            }

            GLOBALTime.totalFrameCount = Win32GetPerformanceCount() - beginFrame;
            GLOBALTime.totalFrameCycles = GetProcessorClockCycles() - beginFrameCycles;
            f32 totalMsPerFrame = PerformanceCountToMilliseconds(GLOBALTime.totalFrameCount);
            f32 framesPerSec = 1 / PerformanceCountToSeconds(GLOBALTime.totalFrameCount); 

            Log("frame = %ffps %lucycles %fms\n", framesPerSec, GLOBALTime.totalFrameCycles, totalMsPerFrame);
            beginFrame = Win32GetPerformanceCount();
            beginFrameCycles = GetProcessorClockCycles();
        }
    }
    
    return(0);
}