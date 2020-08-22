// ------------------------- SETTINGS ------------------------- \\

// RENDERER_OPENGL || RENDERER_SOFTWARE
#define RENDERER_START RENDERER_SOFTWARE

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

// CW_USEDEFAULT == doesnt matter
#define DEFAULT_WINDOW_POS_X 0
#define DEFAULT_WINDOW_POS_Y 0

#define WINDOW_TITLE "PLACEHOLDER"

// 0(invisible) - 255(fully-visible)
#define WINDOW_TRANSPARENCY 255

// ------------------------- SETTINGS ------------------------- \\

// ------------------------- GENERAL ------------------------- \\

#include <stdint.h>
#include <assert.h>

typedef char     str8;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef int8_t   bool8;
typedef int16_t  bool16;
typedef int32_t  bool32;

#define global_variable static
// Function internal to the obj, file
#define internal static 
// WARNING(KKrzosa): Only for .CPP
#define external extern "C"

#define true 1
#define false 0

#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)

#define Log(text, ...)         ConsoleLog(text, __VA_ARGS__)
#define LogInfo(text, ...)     ConsoleLogExtra("INFO:    ", text, __VA_ARGS__)
#define LogSuccess(text, ...)  ConsoleLogExtra("SUCCESS: ", text, __VA_ARGS__)
#define LogError(text, ...)    \
ConsoleLogExtra("ERROR %s %s %d: ", text, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define Assert(expression)     if(!(expression)) PrivateSetDebuggerBreakpoint("Assert") 
#define Error(text)            PrivateSetDebuggerBreakpoint(text)
#define dbg()                  PrivateSetDebuggerBreakpoint("BREAKPOINT") 

typedef union v2
{
    struct
    {
        f32 x;
        f32 y;
    };
    struct 
    {
        f32 u;
        f32 v;
    };
    struct
    {
        f32 width;
        f32 height;
    };
} v2;

typedef union iv2
{
    struct
    {
        i32 x;
        i32 y;
    };
    struct
    {
        i32 width;
        i32 height;
    };
} iv2;

// ------------------------- OS_INTERFACE_HEADER ------------------------- \\

typedef struct memory_storage
{
    void *memory;
    u64 maxSize;
    u64 allocatedSize;
    u64 highestAllocatedSize;
} memory_storage;

typedef struct file_data
{
    void *contents;
    u32 size;
} file_data;

typedef enum keyboard_keys
{
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F12,
    KEY_ESC,
    
    KEY_COUNT,
} keyboard_keys;

typedef enum controller_buttons
{
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    
    BUTTON_DPAD_UP,
    BUTTON_DPAD_DOWN,
    BUTTON_DPAD_LEFT,
    BUTTON_DPAD_RIGHT,
    
    BUTTON_LEFT_SHOULDER,
    BUTTON_RIGHT_SHOULDER,
    
    BUTTON_START,
    BUTTON_SELECT,
    
    BUTTON_COUNT,
} controller_buttons;

typedef struct user_input_controller
{
    f32 leftStickX;
    f32 leftStickY;
    
    f32 rightStickX;
    f32 rightStickY;
    
    bool8 connected;
    bool8 currentButtonState[BUTTON_COUNT];
    bool8 previousButtonState[BUTTON_COUNT];
} user_input_controller;

typedef struct user_input_keyboard
{
    bool8 currentKeyState[KEY_COUNT];
    bool8 previousKeyState[KEY_COUNT];
} user_input_keyboard;

typedef struct user_input_mouse
{
    i32 mousePosX;
    i32 mousePosY;
    i32 mouseWheel;
    
    bool8 left;
    bool8 right;
    bool8 middle;
} user_input_mouse;

typedef struct user_input
{
    user_input_controller controller[4];
    i32 selectedController;
    
    user_input_keyboard keyboard;
    user_input_mouse mouse;
} user_input;

typedef struct time_data
{
    // READ ONLY
    // NOTE: count is a very granular unit of time
    // countsPerSecond says how many counts there are per second
    // I would advice to only use counts for debugging 
    i64 countsPerSecond;
    
    // TimeStamp taken at the program start
    // Cycles as in processor clock cycles
    u64 startAppCycles;
    u64 startAppCount;
    f32 startAppMilliseconds;
    
    // Length of the update, without the sleep 
    u64 updateCycles;
    i64 updateCount;
    f32 updateMilliseconds;
    
    // Length of the update, with sleep
    u64 frameCycles;
    i64 frameCount;
    f32 frameMilliseconds;
} time_data;

typedef struct file_contents
{
    i8 *file;
    u64 fileSize;
    char fileName[260];
    u32 fileNameLength;
} file_contents;

typedef struct files
{
    file_contents files[255];
    u32 fileCount;
    u64 memoryFilled;
} files;

typedef enum enum_renderer
{
    RENDERER_OPENGL,
    RENDERER_SOFTWARE,
} enum_renderer;

typedef struct graphics_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, memory Order BB GG RR XX
    u8 *memory;
    iv2 size;
    i32 pitch;
} graphics_buffer;

typedef struct operating_system_interface
{
    memory_storage pernamentStorage;
    memory_storage temporaryStorage;
    
    str8 *pathToExecutableDirectory;
    
    // NOTE: this buffer should be filled from the very beginning on every frame
    // by the ammount specified in requestedSamples, one sample should be 16bits
    // one frame (left right samples) should be 32 bits
    // this buffer is cleared on the os side on every frame
    void *audioBuffer; 
    u32 audioBufferSize;
    
    // NOTE: number of samples to fill requested from the os
    u32 requestedSamples; 
    u32 samplesPerSecond;
    // NOTE: // this value controls the audio latency, bigger number, bigger latency
    f32 audioLatencyMultiplier; 
    
    // Should be accessed through functions
    // NOTE: user input info, accessed through IsKeyDown etc.
    user_input userInput;
    // NOTE: update time, frame time, app start time
    time_data timeData;
    
    // NOTE: you can change fps by changing this value
    f32 targetFramesPerSecond;
    enum_renderer currentRenderer;
    
    // NOTE(KKrzosa): For software rendering
    graphics_buffer graphicsBuffer;
    
    void   (*Quit)();
    void   (*Log)(char *text, ...);
    void   (*LogExtra)(char *prepend, char *text, ...);
    
    f32    (*TimeGetMilliseconds)();
    i64    (*TimeGetCounts)();
    u64    (*TimeGetProcessorCycles)();
    
    i64    (*FileGetSize)(char *filename); 
    // NOTE: returns bytesRead
    u64    (*FileRead)(char *filename, void *memoryToFill, u64 maxBytesToRead);
    files  (*DirectoryReadAllFiles)(char *directory, void *memoryToFill, u64 maxBytesToRead);
    
    bool32 (*VSyncGetState)();
    bool32 (*VSyncSetState)(bool32 state);
    f32    (*MonitorGetRefreshRate)();
    
    iv2    (*WindowGetSize)();
    void   (*WindowSetTransparency)(u8 value);
    void   (*WindowAlwaysOnTop)();
    void   (*WindowNotAlwaysOnTop)();
    void   (*WindowSetSize)(i32 width, i32 height);
    void   (*WindowSetPosition)(i32 width, i32 height);
    void   (*WindowDrawBorder)(bool32 draw);
    
    void  *(*OpenGLLoadProcedures)(char *name);
} operating_system_interface;

// ------------------------- OS_INTERFACE_HEADER ------------------------- \\

// ------------------------- OS_INTERFACE ------------------------- \\

#if defined(OS_INTERFACE_IMPLEMENTATION)

#if _MSC_VER
#define SilentSetDebuggerBreakpoint() {__debugbreak();}
#else
#define SilentSetDebuggerBreakpoint() { *(volatile int *)0 = 0;}
#endif

global_variable operating_system_interface *PrivateOSPointer = 0;

internal void
AttachOS(operating_system_interface *os)
{
    assert(os != 0);
    PrivateOSPointer = os;
}

internal operating_system_interface *
GetOS()
{
    assert(PrivateOSPointer != 0);
    return PrivateOSPointer;
}

internal bool32
IsKeyPressedOnce(keyboard_keys KEY)
{
    operating_system_interface *os = GetOS();
    
    if(os->userInput.keyboard.previousKeyState[KEY] == 0 &&
       os->userInput.keyboard.currentKeyState[KEY] == 1)
    {
        os->userInput.keyboard.previousKeyState[KEY] = 1;
        return true;
    }
    return false;
}

internal bool32
IsKeyUnpressedOnce(keyboard_keys KEY)
{
    operating_system_interface *os = GetOS();
    if(os->userInput.keyboard.previousKeyState[KEY] == 1 &&
       os->userInput.keyboard.currentKeyState[KEY] == 0)
    {
        os->userInput.keyboard.previousKeyState[KEY] = 0;
        return true;
    }
    return false;
}

internal bool32
IsKeyDown(keyboard_keys KEY)
{
    operating_system_interface *os = GetOS();
    if(os->userInput.keyboard.currentKeyState[KEY] == 1)
    {
        return true;
    }
    return false;
}

internal bool32
IsKeyUp(keyboard_keys KEY)
{
    operating_system_interface *os = GetOS();
    
    if(os->userInput.keyboard.currentKeyState[KEY] == 0)
    {
        return true;
    }
    return false;
}

internal void
SelectActiveController(i32 index)
{
    operating_system_interface *os = GetOS();
    os->userInput.selectedController = index;
}

internal bool32
IsButtonPressedOnce(controller_buttons BUTTON)
{
    operating_system_interface *os = GetOS();
    
    i32 c = os->userInput.selectedController;
    if(os->userInput.controller[c].previousButtonState[BUTTON] == 0 &&
       os->userInput.controller[c].currentButtonState[BUTTON] == 1)
    {
        os->userInput.controller[c].previousButtonState[BUTTON] = 1;
        return true;
    }
    return false;
}

internal bool32
IsButtonUnpressedOnce(controller_buttons BUTTON)
{
    operating_system_interface *os = GetOS();
    
    i32 c = os->userInput.selectedController;
    if(os->userInput.controller[c].previousButtonState[BUTTON] == 1 &&
       os->userInput.controller[c].currentButtonState[BUTTON] == 0)
    {
        os->userInput.controller[c].previousButtonState[BUTTON] = 0;
        return true;
    }
    return false;
}

internal bool32
IsButtonDown(controller_buttons BUTTON)
{
    operating_system_interface *os = GetOS();
    
    i32 c = os->userInput.selectedController;
    if(os->userInput.controller[c].currentButtonState[BUTTON] == 1)
    {
        return true;
    }
    return false;
}

internal bool32
IsButtonUp(controller_buttons BUTTON)
{
    operating_system_interface *os = GetOS();
    
    i32 c = os->userInput.selectedController;
    if(os->userInput.controller[c].currentButtonState[BUTTON] == 0)
    {
        return true;
    }
    return false;
}

internal v2
ControllerLeftStick()
{
    operating_system_interface *os = GetOS();
    
    i32 c = os->userInput.selectedController;
    v2 result = {os->userInput.controller[c].leftStickX, os->userInput.controller[c].leftStickY};
    
    return result;
}

internal v2
ControllerRightStick()
{
    operating_system_interface *os = GetOS();
    
    i32 c = os->userInput.selectedController;
    v2 result = {os->userInput.controller[c].rightStickX, os->userInput.controller[c].rightStickY};
    
    return result;
}

// TODO(KKrzosa): Make it so that we dont have to zero the mousewhell status
// NOTE(KKrzosa): MouseWheel status is zeroed after the retrival
internal i32
MouseWheelStatus()
{
    operating_system_interface *os = GetOS();
    i32 result = os->userInput.mouse.mouseWheel;
    os->userInput.mouse.mouseWheel = 0;
    return result;
}

internal i32
MousePositionX()
{
    operating_system_interface *os = GetOS();
    return os->userInput.mouse.mousePosX;
}

internal i32
MousePositionY()
{
    operating_system_interface *os = GetOS();
    return os->userInput.mouse.mousePosY;
}

internal iv2
MousePosition()
{
    operating_system_interface *os = GetOS();
    iv2 result = {os->userInput.mouse.mousePosX, os->userInput.mouse.mousePosY};
    return result;
}

internal f32
AppStartTimeMilliseconds()
{
    operating_system_interface *os = GetOS();
    return os->timeData.startAppMilliseconds;
}

internal i64 
AppStartTimeCounts()
{
    operating_system_interface *os = GetOS();
    return os->timeData.startAppCount;
}

internal i64 
AppStartTimeCycles()
{
    operating_system_interface *os = GetOS();
    return os->timeData.startAppCycles;
}

//
// NOTE: Update == how long it took to process all the things in a frame without the 
//                 end frame sleep

internal f32
UpdateTimeMilliseconds()
{
    operating_system_interface *os = GetOS();
    return os->timeData.updateMilliseconds;
}

internal i64 
UpdateTimeCounts()
{
    operating_system_interface *os = GetOS();
    return os->timeData.updateCount;
}

internal i64 
UpdateTimeCycles()
{
    operating_system_interface *os = GetOS();
    return os->timeData.updateCycles;
}

// NOTE: Frame == entire length of a single frame

internal f32
FrameTimeMilliseconds()
{
    operating_system_interface *os = GetOS();
    return os->timeData.updateMilliseconds;
}

internal i64 
FrameTimeCounts()
{
    operating_system_interface *os = GetOS();
    return os->timeData.updateCount;
}

internal i64 
FrameTimeCycles()
{
    operating_system_interface *os = GetOS();
    return os->timeData.updateCycles;
}

inline internal f32
MillisecondsToFramesPerSecond(f32 millisecondsPerFrame)
{
    return (1 / millisecondsPerFrame) * 1000;
}


#define ConsoleLog(text, ...) GetOS()->Log(text, __VA_ARGS__)
#define ConsoleLogExtra(prepend, text, ...) GetOS()->LogExtra(prepend, text, __VA_ARGS__)

// ------------------------- OS_INTERFACE ------------------------- \\

// ------------------------- LOADING OPENGL FUNCTIONS ------------------------- \\

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef far
#undef near
#include <gl/GL.h>
#include "opengl_headers/wglext.h"
#else
#error "ERROR: only windows supported"
#endif

#include "opengl_headers/glext.h"

// NOTE: OPENGL Pointers to functions
//       expands to, for example PFNGLBUFFERDATAPROC glBufferData;
#define GLLoad(name, type) static PFNGL##type##PROC gl##name;
// NOTE: Load OpenGL function prototypes from file
#include "opengl_procedures.include"
#undef GLLoad // undefine GLLoad macro

internal void
OpenGLLoadProcedures(void *(*OpenGLLoadProcedures)(char *name))
{
    // NOTE: Expands to, for example gl.UseProgram = (PFNGLUSEPROGRAMPROC)Win32OpenGLLoadProcedures("glUseProgram");
#define GLLoad(name, type) gl##name = (PFNGL##type##PROC)OpenGLLoadProcedures("gl" #name);
    
    // NOTE: Load main OpenGL functions using a macro
    // Expands to glUseProgram = (PFNGLUSEPROGRAMPROC)Win32OpenGLLoadProcedures("glUseProgram");
#include "opengl_procedures.include" // include OpenGL functions to load
#undef GLLoad // undefine GLLoad macro 
    
    LogSuccess("OpenGL functions loaded");
}
#endif

// ------------------------- LOADING OPENGL FUNCTIONS ------------------------- \\

// ------------------------- ------------------------- \\

#if _MSC_VER
#define PrivateSetDebuggerBreakpoint(text) {LogError(text); __debugbreak();}
#define SilentSetDebuggerBreakpoint() {__debugbreak();}
#else
#define PrivateSetDebuggerBreakpoint(text) {LogError(text); *(volatile int *)0 = 0;}
#define SilentSetDebuggerBreakpoint() { *(volatile int *)0 = 0;}
#endif

#if defined(WIN32_EXE)
// CStandard Lib and Windows
#include <windows.h>
#include <xinput.h>
#include <intrin.h>
#include <stdio.h>
#include <memory.h>

// OpenGL
#include <gl/GL.h>
#include "opengl_headers/wglext.h"
#include "opengl_headers/glext.h"

// NOTE: These global variables are used in functions
// that are passed to the application

typedef struct win32_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
    int bytesPerPixel;
} win32_offscreen_buffer;


// Loop status, the app closes if it equals 0
global_variable bool32 GLOBALApplicationIsRunning; 
global_variable bool32 GLOBALSleepIsGranular;
global_variable i64    GLOBALCountsPerSecond;

global_variable f32    GLOBALMonitorRefreshRate;
global_variable bool32 GLOBALVSyncState;

global_variable enum_renderer GLOBALRenderer;
global_variable win32_offscreen_buffer GLOBALGraphicsBuffer;

global_variable HWND   GLOBALWindow;
global_variable HANDLE GLOBALConsoleHandle;

/* TODO: 
 * memory stuff
 * string api that allows for custom memory allocator
 * fullscreen
 * better window resize handling
 * lock the resizing?
 * delete or move to the dll the "keep aspect ratio" thing
 * option to turn off the locking framerate
 * save to file 
 * append to file
 * list all files in a directory
 * make folder
 * does file exist
*/

//------------------------- STRINGS -------------------------\\

#define STR_ALLOC(size) malloc(size)

typedef struct string_header
{
    size_t length;
    size_t capacity;
} string_header;

inline static size_t
CharLength(char *text)
{
    size_t stringSize = 0;
    for(size_t i = 0; text[i] != '\0'; i++)
        stringSize++;
    return stringSize;
}

inline static size_t 
StringLength(str8 *string)
{
    return ((string_header *)(string) - 1)->length;
}

inline static size_t 
StringCapacity(str8 *string)
{
    return ((string_header *)(string) - 1)->capacity;
}

inline static string_header *
StringGetHeader(str8 *string)
{
    return (string_header *)(string) - 1;
}

static str8 *
StringAllocate(size_t size)
{
    string_header *newstring_header = 
        (string_header *)STR_ALLOC(sizeof(string_header) + sizeof(str8) * size + 1);
    str8 *newString = (str8 *)(newstring_header + 1);
    
    newstring_header->length = size;
    newstring_header->capacity = size + 1;
    newString[size] = '\0';
    
    return newString;
}

static str8 *
StringCreate(char *text)
{
    if(text == 0) return 0;
    
    size_t charLength = CharLength(text);
    str8 *newString = StringAllocate(charLength);
    
    memcpy(newString, text, charLength);
    
    return newString;
}

static void
StringFree(str8 *string)
{
    string_header *stringHeader = ((string_header *)string) - 1;
    free(stringHeader);
}

static str8 *
StringCreateSubstring(char *text, size_t length)
{
    size_t textSize = 0;
    while(textSize != length && text[textSize] != '\0')
    {
        textSize++;
    }
    str8 *newString = StringAllocate(textSize);
    
    memcpy(newString, text, textSize);
    
    return newString;
}

static str8 *
StringConcatChar(str8 *string, char *text)
{
    size_t stringLength = StringLength(string);
    size_t charLength = CharLength(text);
    str8 *newString = StringAllocate(stringLength + charLength);
    
    memcpy(newString, string, stringLength);
    memcpy(newString + stringLength, text, charLength);
    
    return newString;
}

static str8 *
StringsConcat(str8 *string1, str8 *string2)
{
    size_t string1Length = StringLength(string1);
    size_t string2Length = StringLength(string2);
    
    str8 *newString = StringAllocate(string1Length + string2Length);
    
    memcpy(newString, string1, string1Length);
    memcpy(newString + string1Length, string2, string2Length);
    
    return newString;
}

//
// CONSOLE
//

#define TEXT_BUFFER_SIZE 1024

internal void 
ConsoleLog(char *text, ...)
{
    static char GLOBALRandomAccessTextBuffer[1][TEXT_BUFFER_SIZE];
    
    va_list args;
    va_start(args, text);
    vsprintf_s(GLOBALRandomAccessTextBuffer[0], TEXT_BUFFER_SIZE, text, args);
    va_end(args);
    
    size_t textLength = CharLength(GLOBALRandomAccessTextBuffer[0]);
    WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer[0], (DWORD)textLength, 0, 0);
}

internal void
ConsoleLogExtra(char *prepend, char *text, ...)
{
    static char GLOBALRandomAccessTextBuffer[2][TEXT_BUFFER_SIZE];
    
    size_t textLength = CharLength(text);
    size_t prependLength = CharLength(prepend);
    if(textLength + prependLength > TEXT_BUFFER_SIZE)
    {
        MessageBoxA(0, "Text buffer overflow", "ERROR", MB_OK);
        OutputDebugStringA("Text buffer overflow!\n");
        SilentSetDebuggerBreakpoint();
    }
    
    memcpy(GLOBALRandomAccessTextBuffer[1], prepend, prependLength);
    memcpy(GLOBALRandomAccessTextBuffer[1] + prependLength, text, textLength);
    GLOBALRandomAccessTextBuffer[1][prependLength + textLength] = '\0';
    
    va_list args;
    va_start(args, text);
    vsprintf_s(GLOBALRandomAccessTextBuffer[0], TEXT_BUFFER_SIZE, 
               GLOBALRandomAccessTextBuffer[1], args);
    va_end(args);
    
    textLength = CharLength(GLOBALRandomAccessTextBuffer[0]);
    GLOBALRandomAccessTextBuffer[0][textLength] = '\n';
    
    WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer[0], (DWORD)(textLength + 1), 0, 0);
}


// NOTE: Attaches to the console that invoked the application
//       if that fails it allocates a new console
internal void
Win32ConsoleAttach(void)
{
    // NOTE: Console Setup
    if(!AttachConsole(ATTACH_PARENT_PROCESS)) 
    {
        OutputDebugStringA("Failed to attach to console\n");
        if(!AllocConsole())
        {
            OutputDebugStringA("Failed to create a console\n");
        }
    }
    
    GLOBALConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(!GLOBALConsoleHandle) OutputDebugStringA("Failed to get console handle\n");
}

//
// TIME
//

#define ProcessorClockCycles() __rdtsc()

inline internal i64
Win32PerformanceCountGet()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return count.QuadPart;
}

inline internal f32
Win32SecondsGet()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return ((f32)count.QuadPart / (f32)GLOBALCountsPerSecond);
}

inline internal f32
Win32MillisecondsGet()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return ((f32)(count.QuadPart * 1000) / (f32)GLOBALCountsPerSecond);
}

// NOTE: Frequency = the number of counts per second
inline internal i64
Win32PerformanceFrequencyGet()
{
    LARGE_INTEGER countsPerSecondResult;
    QueryPerformanceFrequency(&countsPerSecondResult);
    return countsPerSecondResult.QuadPart;
}

inline internal f32
PerformanceCountToMilliseconds(i64 count)
{
    f32 result = (f32)(count * 1000.0f) / (f32)GLOBALCountsPerSecond;
    return result;
}

inline internal f32
PerformanceCountToSeconds(i64 count)
{
    f32 result = (f32)count / (f32)GLOBALCountsPerSecond;
    return result;
}

inline internal f32
PerformanceCountToFramesPerSecond(i64 count)
{
    f32 result = 1 / ((f32)count / (f32)GLOBALCountsPerSecond);
    return result;
}

inline internal f32
MillisecondsPerFrameToFramesPerSecond(f32 millisecondsPerFrame)
{
    return (1 / millisecondsPerFrame) * 1000;
}

internal void
EndFrameAndSleep(time_data *time, f32 targetMsPerFrame, i64 *prevFrame, u64 *prevFrameCycles)
{
    //
    // NOTE: Time the frame and sleep to hit target framerate
    //
    time->updateCount = Win32PerformanceCountGet() - *prevFrame;
    time->updateCycles = ProcessorClockCycles() - *prevFrameCycles;
    time->updateMilliseconds = PerformanceCountToMilliseconds(time->updateCount);
    
    time->frameMilliseconds = time->updateMilliseconds;
    if(time->frameMilliseconds < targetMsPerFrame)
    {
        if(GLOBALSleepIsGranular)
        {
            // TODO: Test on varied frame rate
            f32 sleepOffset = targetMsPerFrame / 8.f;
            f32 timeToSleep = (targetMsPerFrame - time->frameMilliseconds) - sleepOffset;
            if(timeToSleep > 0)
            {
                Sleep((DWORD)timeToSleep);
            }
        }
        
        // NOTE: report if slept too much
        time->frameCount = Win32PerformanceCountGet() - *prevFrame;
        time->frameMilliseconds = PerformanceCountToMilliseconds(time->frameCount);
        if(time->frameMilliseconds > targetMsPerFrame) 
        {
            LogInfo("Slept too much!");
        }
        
        // NOTE: stall if we didnt hit the final ms per frame
        while(time->frameMilliseconds < targetMsPerFrame)
        {
            time->frameCount = Win32PerformanceCountGet() - *prevFrame;
            time->frameMilliseconds = PerformanceCountToMilliseconds(time->frameCount);
        }
    }
    else
    {
        LogInfo("Missed framerate!");
    }
    
    time->frameCount = Win32PerformanceCountGet() - *prevFrame;
    time->frameMilliseconds = PerformanceCountToMilliseconds(time->frameCount);
    time->frameCycles = ProcessorClockCycles() - *prevFrameCycles;
    
    LogInfo("%f frame %f update", time->frameMilliseconds, time->updateMilliseconds);
    *prevFrameCycles = ProcessorClockCycles();
    *prevFrame = Win32PerformanceCountGet();
}

//
// OPENGL
//

// NOTE: Pointers to windows opengl functions
internal PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
internal PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
internal PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB = 0;
internal PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 0;

// NOTE: forward declarations
internal void *Win32OpenGLLoadProcedures(char *name);
internal void PrintLastErrorMessage(char *text);
internal HGLRC Win32OpenGLInit(HDC deviceContext);

internal void * 
Win32OpenGLLoadProcedures(char *name)
{
    void *p = (void *)wglGetProcAddress(name);
    Assert(p);
    
    return p;
}

// Pass 1 to enable vsync
internal HGLRC
Win32OpenGLInit(HDC deviceContext)
{
    HGLRC mainOpenglContext;
    PIXELFORMATDESCRIPTOR pixelFormat =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    i32 pixelFormatIndex = ChoosePixelFormat(deviceContext, &pixelFormat);
    
    if(!pixelFormatIndex)
        Log("FAILED to choose pixel format\n");
    
    if(!SetPixelFormat(deviceContext, pixelFormatIndex, &pixelFormat))
        LogError("FAILED: to set PixelFormat");
    
    HGLRC dummyOpenglContext = wglCreateContext(deviceContext);
    
    if(!dummyOpenglContext) LogError("FAILED: to create dummy opengl context");
    if(!wglMakeCurrent(deviceContext, dummyOpenglContext)) LogError("FAILED: to make dummy opengl context current");
    
    // NOTE: Load windows opengl functions
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)Win32OpenGLLoadProcedures("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)Win32OpenGLLoadProcedures("wglCreateContextAttribsARB");
    wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)Win32OpenGLLoadProcedures("wglMakeContextCurrentARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)Win32OpenGLLoadProcedures("wglSwapIntervalEXT");
    
    int attribList[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0
    };
    
    UINT numFormats = 0;
    wglChoosePixelFormatARB(deviceContext, attribList, 
                            0, 1, &pixelFormatIndex, &numFormats);
    
    if(pixelFormatIndex)
    {
        const int contextAttribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            0
        };
        
        mainOpenglContext = wglCreateContextAttribsARB(deviceContext, dummyOpenglContext, contextAttribs);
        if(mainOpenglContext)
        {
            wglMakeCurrent(deviceContext, 0);
            wglDeleteContext(dummyOpenglContext);
            wglMakeCurrent(deviceContext, mainOpenglContext);
        }
    }
    
    return mainOpenglContext;
    
}

internal iv2
Win32WindowDrawAreaGetSize()
{
    RECT ClientRect;
    iv2 drawArea;
    
    // NOTE: get size of the window, without the border
    GetClientRect(GLOBALWindow, &ClientRect);
    drawArea.width = ClientRect.right - ClientRect.left;
    drawArea.height = ClientRect.bottom - ClientRect.top;
    
    return drawArea;
}

internal void
Win32OpenGLAspectRatioUpdate(i32 ratioWidth, i32 ratioHeight)
{
    iv2 win = Win32WindowDrawAreaGetSize();
    
    // NOTE: keep aspect ratio of 16:9
    i32 transformedWidth = win.height * ratioWidth / ratioHeight;
    i32 centerTheThing = (win.width - transformedWidth) / 2;
    
    glViewport(centerTheThing, 0, transformedWidth, win.height);
}

// NOTE: Vsync is an openGL extension so it can fail
//       @returns true if success 
internal bool32
Win32OpenGLSetVSync(bool32 state)
{
    bool32 result = false;
    if(wglSwapIntervalEXT)
    {
        GLOBALVSyncState = state;
        wglSwapIntervalEXT(state);
        result = true;
    }
    
    return result;
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int width, int height)
{
    // TODO(casey): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.
    
    if(Buffer->memory)
    {
        VirtualFree(Buffer->memory, 0, MEM_RELEASE);
    }
    
    Buffer->width = width;
    Buffer->height = height;
    
    int bytesPerPixel = 4;
    Buffer->bytesPerPixel = bytesPerPixel;
    
    // NOTE(casey): When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    Buffer->info.bmiHeader.biSize = sizeof(Buffer->info.bmiHeader);
    Buffer->info.bmiHeader.biWidth = Buffer->width;
    Buffer->info.bmiHeader.biHeight = -Buffer->height;
    Buffer->info.bmiHeader.biPlanes = 1;
    Buffer->info.bmiHeader.biBitCount = 32;
    Buffer->info.bmiHeader.biCompression = BI_RGB;
    
    // NOTE(casey): Thank you to Chris Hecker of Spy Party fame
    // for clarifying the deal with StretchDIBits and BitBlt!
    // No more DC for us.
    int BitmapMemorySize = (Buffer->width*Buffer->height)*bytesPerPixel;
    Buffer->memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Buffer->pitch = width*bytesPerPixel;
    
    // TODO(casey): Probably clear this to black
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer,
                           HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    // TODO(casey): Aspect ratio correction
    // TODO(casey): Play with stretch modes
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer->width, Buffer->height,
                  Buffer->memory,
                  &Buffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
}

//
// FILEIO
//

// returns 0 on error
internal u64
Win32FileGetSize(char *filename)
{
    Assert(CharLength(filename) < MAX_PATH); // MAX PATH
    
    LARGE_INTEGER size;
    
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        size.QuadPart = 0;
        LogError("INVALID HANDLE VALUE %s", filename);
        return size.QuadPart;
    }
    bool32 result = GetFileSizeEx(fileHandle, &size);
    if(!result)
    {
        size.QuadPart = 0;
        LogError("GetFileSize failed %s", filename);
        return size.QuadPart;
    }
    
    CloseHandle(fileHandle);
    return size.QuadPart;
}

// Fills the specified memory buffer with the file contents
// returns 0 on fail, else returns bytesRead
internal u64
Win32FileRead(char *filename, void *memory, u64 bytesToRead)
{
    Assert(CharLength(filename) < MAX_PATH); // MAX PATH
    
    // OPEN EXISTING flag doesnt create a file
    // Creates a file if it doesnt exist
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0,
                                    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    
    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        LogError("INVALID HANDLE VALUE");
        return 0;
    }
    
    DWORD bytesRead;
    bool32 result = ReadFile(fileHandle, memory, (DWORD)bytesToRead, &bytesRead, 0);
    if(!result)
    {
        LogError("ReadFile failed %s", filename);
        return 0;
    }
    if(bytesRead != bytesToRead)
    {
        LogError("BytesRead and BytesToRead mismatch %s", filename);
        return 0;
    }
    
    LogSuccess("%s FILE LOADED", filename);
    return bytesRead;
}

internal files
Win32DirectoryReadAllFiles(char *directory, void *memory, u64 bytesToRead)
{
    files result = {0};
    u64 totalSizeOfFiles = 0;
    WIN32_FIND_DATAA findData;
    
    str8 *query = StringConcatChar(directory, "/*");
    Assert(StringLength(query) < MAX_PATH);
    
    LogInfo("Load all files in directory %s", directory);
    HANDLE handle = FindFirstFileA(query, &findData);
    if(handle == INVALID_HANDLE_VALUE)
    {
        LogError("INVALID HANDLE VALUE %s", directory);
    }
    else
    {
        LogSuccess("FindFirstFileA %s %lld", findData.cFileName, findData.nFileSizeLow);
    }
    
    while(FindNextFileA(handle, &findData) != 0)
    {
        LogSuccess("FindNextFile %s %lld", findData.cFileName, findData.nFileSizeLow);
        if(findData.nFileSizeLow == 0) continue; // skip empty files and ".."
        
        result.files[result.fileCount].file = (i8 *)memory;
        result.files[result.fileCount].file += totalSizeOfFiles;
        
        if(totalSizeOfFiles + findData.nFileSizeLow < bytesToRead)
        {
            str8 *slash = StringConcatChar(directory, "/");
            str8 *filePath = StringConcatChar(slash, findData.cFileName);
            
            // FUNCTION: Copy file name 
            { 
                size_t length = CharLength(findData.cFileName);
                memcpy(result.files[result.fileCount].fileName, findData.cFileName, length);
                result.files[result.fileCount].fileName[length] = '\0';
                result.files[result.fileCount].fileNameLength = (u32)length;
            }
            
            result.files[result.fileCount].fileSize = 
                Win32FileRead(filePath, result.files[result.fileCount].file, findData.nFileSizeLow);
            
            totalSizeOfFiles += result.files[result.fileCount].fileSize;
            result.fileCount++;
            
            StringFree(slash);
            StringFree(filePath);
        }
        else
        {
            LogError("Failed to load some of the files because passed in bytesToRead is too small for all the files to fit");
            break;
        }
    }
    
    DWORD message = GetLastError();
    if(message != ERROR_NO_MORE_FILES) LogError("FindNextFile %lu", message);
    
    FindClose(handle);
    StringFree(query);
    
    result.memoryFilled = totalSizeOfFiles;
    return result;
}

internal str8 *
Win32GetExecutableDirectory()
{
    char fileName[1024];
    
    // if this parameter is NULL, GetModuleFileName retrieves the path of the executable file of the current process.
    DWORD result = GetModuleFileNameA(0, fileName, 1024);
    if(!result)
    {
        LogError("GetModuleFileNameA");
    }
    
    i32 i = result - 1;
    for(; i != 0; i--)
    {
        if(fileName[i] == '\\') break;
    }
    
    return StringCreateSubstring(fileName, i);
}

//
// INPUT
//

// NOTE: Get mouse position
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

// NOTE: Functions as types
typedef DWORD WINAPI XInputGetStateProc(DWORD dw_user_index, XINPUT_STATE *p_state);
typedef DWORD WINAPI XInputSetStateProc(DWORD dw_user_index, XINPUT_VIBRATION *p_vibration);

// NOTE: Empty (stub) functions as replacements
DWORD WINAPI XInputGetStateStub(DWORD dw_user_index, XINPUT_STATE *p_state){return ERROR_DEVICE_NOT_CONNECTED;}
DWORD WINAPI XInputSetStateStub(DWORD dw_user_index, XINPUT_VIBRATION *p_vibration){return ERROR_DEVICE_NOT_CONNECTED;}

// NOTE: Pointers to loaded functions
static XInputSetStateProc *XInputSetStateFunctionPointer = XInputSetStateStub;
static XInputGetStateProc *XInputGetStateFunctionPointer = XInputGetStateStub;

internal void 
Win32XInputLoad(void)    
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    
    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
        Log("xinput1_4.dll load");
    }
    
    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
        LogError("xinput9_1_0.dll load");
    }
    
    if(XInputLibrary)
    {
        XInputGetStateFunctionPointer = (XInputGetStateProc *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetStateFunctionPointer) 
        {
            XInputGetStateFunctionPointer = XInputGetStateStub; 
            LogError("XInput GetState load");
            return;
        }
        
        XInputSetStateFunctionPointer = (XInputSetStateProc *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetStateFunctionPointer) 
        {
            XInputSetStateFunctionPointer = XInputSetStateStub;
            LogError("XInput SetState load");
            return;
        }
        
        LogSuccess("XInput load");
    }
    else
    {
        LogError("XINPUT library load");
    }
}

#define KEYUpdate(KEY){ \
keyboard->previousKeyState[KEY] = keyboard->currentKeyState[KEY]; \
keyboard->currentKeyState[KEY] = isKeyDown;}

#define BUTTONUpdate(BUTTON, XINPUT_BUTTON) \
controller->previousButtonState[BUTTON] = controller->currentButtonState[BUTTON]; \
controller->currentButtonState[BUTTON] = (gamepad->wButtons & XINPUT_BUTTON) != 0;

internal void
Win32InputUpdate(user_input *userInput)
{
    user_input_keyboard *keyboard = &userInput->keyboard;
    user_input_mouse *mouse = &userInput->mouse;
    
    MSG message;
    while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))   
    {
        WPARAM VKCode = message.wParam;
        switch (message.message)
        {
            case WM_KEYUP:
            case WM_KEYDOWN:
            case WM_SYSKEYUP:
            case WM_SYSKEYDOWN:
            {
                bool8 isKeyDown = (message.message == WM_KEYDOWN);
                
                if(VKCode == 'Q') KEYUpdate(KEY_Q)
                    if(VKCode == 'W') KEYUpdate(KEY_W)
                    if(VKCode == 'E') KEYUpdate(KEY_E)
                    if(VKCode == 'R') KEYUpdate(KEY_R)
                    if(VKCode == 'T') KEYUpdate(KEY_T)
                    if(VKCode == 'Y') KEYUpdate(KEY_Y)
                    if(VKCode == 'U') KEYUpdate(KEY_U)
                    if(VKCode == 'I') KEYUpdate(KEY_I)
                    if(VKCode == 'O') KEYUpdate(KEY_O)
                    if(VKCode == 'P') KEYUpdate(KEY_P)
                    if(VKCode == 'A') KEYUpdate(KEY_A)
                    if(VKCode == 'S') KEYUpdate(KEY_S)
                    if(VKCode == 'D') KEYUpdate(KEY_D)
                    if(VKCode == 'F') KEYUpdate(KEY_F)
                    if(VKCode == 'G') KEYUpdate(KEY_G)
                    if(VKCode == 'H') KEYUpdate(KEY_H)
                    if(VKCode == 'J') KEYUpdate(KEY_J)
                    if(VKCode == 'K') KEYUpdate(KEY_K)
                    if(VKCode == 'L') KEYUpdate(KEY_L)
                    if(VKCode == 'Z') KEYUpdate(KEY_Z)
                    if(VKCode == 'X') KEYUpdate(KEY_X)
                    if(VKCode == 'C') KEYUpdate(KEY_C)
                    if(VKCode == 'V') KEYUpdate(KEY_V)
                    if(VKCode == 'B') KEYUpdate(KEY_B)
                    if(VKCode == 'N') KEYUpdate(KEY_N)
                    if(VKCode == 'M') KEYUpdate(KEY_M)
                    if(VKCode == VK_UP) KEYUpdate(KEY_UP)
                    if(VKCode == VK_DOWN) KEYUpdate(KEY_DOWN)
                    if(VKCode == VK_LEFT) KEYUpdate(KEY_LEFT)
                    if(VKCode == VK_RIGHT) KEYUpdate(KEY_RIGHT)
                    if(VKCode == VK_F1) KEYUpdate(KEY_F1)
                    if(VKCode == VK_F2) KEYUpdate(KEY_F2)
                    if(VKCode == VK_F3) KEYUpdate(KEY_F3)
                    if(VKCode == VK_F4) KEYUpdate(KEY_F4)
                    if(VKCode == VK_F12) KEYUpdate(KEY_F12)
                    if(VKCode == VK_ESCAPE) KEYUpdate(KEY_ESC)
                    else{} // NOTE: eat other keys
                break;
            }
            case WM_MOUSEMOVE:
            {
                mouse->mousePosX = GET_X_LPARAM(message.lParam); 
                mouse->mousePosY = GET_Y_LPARAM(message.lParam);
                break;
            }
            case WM_MOUSEWHEEL:
            {
                if((i32)VKCode > 0) mouse->mouseWheel = 1;
                else mouse->mouseWheel = -1;
                
            }
            case WM_LBUTTONUP:
            case WM_LBUTTONDOWN:
            {
                mouse->left = (message.message == WM_LBUTTONDOWN);
                break;
            }
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            {
                mouse->middle = (message.message == WM_MBUTTONDOWN);
                break;
            }
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            {
                mouse->right = (message.message == WM_RBUTTONDOWN);
                break;
            }
            default:
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
                break;
            }
        }
    }
}

internal void
Win32XInputUpdate(user_input *userInput)
{
    // NOTE: i = controller index
    for (DWORD i=0; i < XUSER_MAX_COUNT; i++ )
    {
        XINPUT_STATE state;
        user_input_controller *controller = &userInput->controller[i];
        if(XInputGetStateFunctionPointer( i, &state ) == ERROR_SUCCESS)
        {
            XINPUT_GAMEPAD *gamepad = &state.Gamepad;
            
            BUTTONUpdate(BUTTON_DOWN, XINPUT_GAMEPAD_A)
                BUTTONUpdate(BUTTON_RIGHT, XINPUT_GAMEPAD_B)
                BUTTONUpdate(BUTTON_LEFT, XINPUT_GAMEPAD_X)
                BUTTONUpdate(BUTTON_UP, XINPUT_GAMEPAD_Y)
                
                BUTTONUpdate(BUTTON_DPAD_UP, XINPUT_GAMEPAD_DPAD_UP)
                BUTTONUpdate(BUTTON_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_DOWN)
                BUTTONUpdate(BUTTON_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT)
                BUTTONUpdate(BUTTON_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_LEFT)
                
                BUTTONUpdate(BUTTON_START, XINPUT_GAMEPAD_START)
                BUTTONUpdate(BUTTON_SELECT, XINPUT_GAMEPAD_BACK)
                
                BUTTONUpdate(BUTTON_LEFT_SHOULDER, XINPUT_GAMEPAD_LEFT_SHOULDER)
                BUTTONUpdate(BUTTON_RIGHT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER)
                
#define StickRange 32767.f
#define LeftStickDeadzone 7849
            
            controller->leftStickX = 0; 
            controller->leftStickY = 0;
            
            // NOTE: Take deadzone into account for the left stick
            if(gamepad->sThumbLX > LeftStickDeadzone || 
               gamepad->sThumbLX < -LeftStickDeadzone)
            {
                // NOTE: Normalize the stick values
                controller->leftStickX = gamepad->sThumbLX / StickRange;
            }
            
            if(gamepad->sThumbLY > LeftStickDeadzone ||
               gamepad->sThumbLY < -LeftStickDeadzone)
            {
                // NOTE: Normalize the stick values
                controller->leftStickY = gamepad->sThumbLY / StickRange;
            }
            
#define RightStickDeadzone 8689
            
            controller->rightStickX = 0; 
            controller->rightStickY = 0;
            
            // NOTE: Take deadzone into account for the right stick
            if(gamepad->sThumbRX > RightStickDeadzone || 
               gamepad->sThumbRX < -RightStickDeadzone)
            {
                // NOTE: Normalize the stick values
                controller->rightStickX = gamepad->sThumbRX / StickRange;
            }    
            
            if(gamepad->sThumbRY > RightStickDeadzone ||
               gamepad->sThumbRY < -RightStickDeadzone)
            {
                // NOTE: Normalize the stick values
                controller->rightStickY = gamepad->sThumbRY / StickRange;
            }
            
            
            controller->connected = 1;
        }
        else
        {
            controller->connected = 0;
        }
    }
}

//
// HOTRELOAD
//

// NOTE: prototypes for function pointers
typedef void Initialize(operating_system_interface *memory); // called at the beginning of the app
typedef void HotReload(operating_system_interface *memory); // called on hot reload
typedef void HotUnload(operating_system_interface *memory); // called on hot reload
typedef void Update(operating_system_interface *memory); // called on every frame

// NOTE: empty functions meant to be replacements when
// functions from the dll fail to load
void InitializeStub(operating_system_interface *memory){}
void HotReloadStub(operating_system_interface *memory){}
void HotUnloadStub(operating_system_interface *memory){}
void UpdateStub(operating_system_interface *memory){}

typedef struct win32_dll_code
{
    HMODULE library;    
    FILETIME lastDllWriteTime;
    bool32 isValid;
    
    // NOTE: pointers to functions from the dll
    Initialize *initialize;
    HotReload *hotReload;
    HotUnload *hotUnload;
    Update *update;
} win32_dll_code;

/* Searches for a file, extracts properties and returns the time
    the file was last written to  */
internal FILETIME 
Win32LastWriteTimeGet(char* file)
{
    FILETIME lastWriteTime = {0};
    WIN32_FIND_DATAA data;
    HANDLE dllFileHandle = FindFirstFileA(file, &data);
    if(dllFileHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(dllFileHandle);
        lastWriteTime = data.ftLastWriteTime;
    }
    else
    {
        LogError("Invalid handle value!");
    }
    
    return lastWriteTime;
}

// Creates a copy of the main dll, and loads that copy
// if load fails it substitutes the loaded function with a stub(empty function)
internal win32_dll_code
Win32DLLCodeLoad(char *mainDllPath, char *tempDllPath)
{
    win32_dll_code result;
    result.lastDllWriteTime = Win32LastWriteTimeGet(tempDllPath);
    
    CopyFileA((LPCSTR)mainDllPath, (LPCSTR)tempDllPath, FALSE);
    
    result.library = LoadLibraryA(tempDllPath);
    result.isValid = 1;
    
    // NOTE: Load the functions from the game dll
    if (result.library)
    {
        result.initialize = (Initialize *)GetProcAddress(result.library, "Initialize");
        result.hotReload = (HotReload *)GetProcAddress(result.library, "HotReload");
        result.hotUnload = (HotUnload *)GetProcAddress(result.library, "HotUnload");
        result.update = (Update *)GetProcAddress(result.library, "Update");
        
        result.isValid = (result.update != 0) &&
            (result.hotReload != 0) && 
            (result.hotUnload != 0) && 
            (result.initialize != 0);
    }
    
    // NOTE: if functions failed to load, load the stubs
    if (result.isValid == 0)
    {
        result.update = UpdateStub;
        result.initialize = InitializeStub;
        result.hotReload = HotReloadStub;
        result.hotUnload = HotUnloadStub;
        
        LogError("MainDLLCode Load");
    }
    
    LogInfo("MainDLLCode Load");
    return result;
}

/* Unloads the dll and nulls the pointers to functions from the dll */
internal void
Win32DLLCodeUnload(win32_dll_code *dllCode)
{
    if (dllCode->library)
    {
        FreeLibrary(dllCode->library);
        dllCode->library = 0;
        dllCode->initialize = InitializeStub;
        dllCode->hotReload = HotReloadStub;
        dllCode->hotUnload = HotReloadStub;
        dllCode->update = UpdateStub;
        LogInfo("MainDLLCode Unload");
    }
    
    dllCode->isValid = false;
}

internal void
Win32UpdateDLLCode(win32_dll_code *dllCode, char *mainDLLPath, char *tempDLLPath, operating_system_interface *os)
{
    // NOTE: Check if dll was rebuild and load it again if it did
    FILETIME newDLLWriteTime = Win32LastWriteTimeGet(mainDLLPath);
    if(CompareFileTime(&newDLLWriteTime, &dllCode->lastDllWriteTime) != 0)
    {
        dllCode->hotUnload(os);
        
        Win32DLLCodeUnload(dllCode);
        Sleep(100);
        
        *dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);
        
        // NOTE: Call HotReload function from the dll
        dllCode->hotReload(os);
    }
}

//
// AUDIO WASAPI
//

#include <objbase.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>

static const GUID IID_IAudioClient = {0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2};
static const GUID IID_IAudioRenderClient = {0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2};
static const GUID CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E};
static const GUID IID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6};
static const GUID PcmSubformatGuid = {STATIC_KSDATAFORMAT_SUBTYPE_PCM};

// NOTE: typedefines for the functions which are goint to be loaded
typedef HRESULT CoCreateInstanceFunction(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
typedef HRESULT CoInitializeExFunction(LPVOID pvReserved, DWORD dwCoInit);

// NOTE: empty functions(stubs) which are used when library fails to load
HRESULT CoCreateInstanceStub(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv) { return 0; }
HRESULT CoInitializeExStub(LPVOID pvReserved, DWORD dwCoInit) { return 0; }

// NOTE: pointers to the functions from the dll
CoCreateInstanceFunction *CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
CoInitializeExFunction *CoInitializeExFunctionPointer = CoInitializeExStub;

// NOTE: Number of REFERENCE_TIME units per second
// One unit is equal to 100 nano seconds
#define REF_TIMES_PER_SECOND 10000000

typedef struct win32_audio_data
{
    IMMDevice *device;
    IAudioClient *audioClient;
    IMMDeviceEnumerator *deviceEnum;
    IAudioRenderClient *audioRenderClient;
    
    u32 samplesPerSecond;
    u32 numberOfChannels;
    
    // NOTE: one frame is 2 samples (left, right) 32 bits if
    // one sample is equal 16 bit
    u32 bufferFrameCount; 
    u32 latencyFrameCount;
    i32 bitsPerSample;
    REFERENCE_TIME bufferDuration;
    bool32 initialized;
} win32_audio_data;

internal inline u32
AudioBufferSize(win32_audio_data audioData)
{
    return audioData.bufferFrameCount * (audioData.bitsPerSample * 2);
}

// NOTE: Load COM Library functions dynamically, 
//       this way sound is not necessary to run the game
internal void
Win32COMLoad(void)
{
    HMODULE ole32Library = LoadLibraryA("ole32.dll");
    if (ole32Library)
    {
        LogSuccess("COM Ole32.dll Loaded");
        CoCreateInstanceFunctionPointer = 
            (CoCreateInstanceFunction *)GetProcAddress(ole32Library, "CoCreateInstance");
        
        if (!CoCreateInstanceFunctionPointer)
        {
            LogError("CoCreateInstance load");
            CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
        }
        CoInitializeExFunctionPointer = 
            (CoInitializeExFunction *)GetProcAddress(ole32Library, "CoInitializeEx");
        
        if (!CoInitializeExFunctionPointer)
        {
            LogError("CoInitializeEx load");
            CoInitializeExFunctionPointer = CoInitializeExStub;
        }
    }
    else
    {
        LogError("COM Ole32.dll load");
        CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
        CoInitializeExFunctionPointer = CoInitializeExStub;
    }
}

// NOTE: Bigger number, smaller latency 
internal win32_audio_data
Win32AudioInitialize(i32 samplesPerSecond)
{
    Win32COMLoad();
    
    win32_audio_data audio = {0};
    
    HRESULT result;
    result = CoInitializeExFunctionPointer(0, COINIT_SPEED_OVER_MEMORY);
    
    if(result != S_OK)
    {
        LogError("CoInitializeExFunction");
        return audio;
    }
    
    result = CoCreateInstanceFunctionPointer(
                                             &CLSID_MMDeviceEnumerator, NULL,
                                             CLSCTX_ALL, &IID_IMMDeviceEnumerator,
                                             (LPVOID *)&audio.deviceEnum);
    
    if (result != S_OK)
    {
        LogError("CoCreateInstance");
        return audio;
    }
    
    result = audio.deviceEnum->lpVtbl->GetDefaultAudioEndpoint(
                                                               audio.deviceEnum, eRender, eConsole, &audio.device
                                                               );
    
    if (result != S_OK)
    {
        LogError("GetDefaultAudioEndpoint");
        return audio;
    }
    
    result = audio.device->lpVtbl->Activate(
                                            audio.device, &IID_IAudioClient, CLSCTX_ALL, 0, (void **)&audio.audioClient
                                            );
    
    if (result != S_OK)
    {
        LogError("IAudioClient Activate");
        return audio;
    }
    
    // WAVEFORMATEX *currWaveFormat = 0;
    // audioClient->lpVtbl->GetMixFormat(audioClient, &currWaveFormat);
    
    audio.bitsPerSample = 16;
    audio.numberOfChannels = 2;
    audio.samplesPerSecond = samplesPerSecond;
    WAVEFORMATEX waveFormat = {0};
    {
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = audio.numberOfChannels;
        waveFormat.nSamplesPerSec = audio.samplesPerSecond;
        waveFormat.wBitsPerSample = audio.bitsPerSample;
        waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize = 0;
    }
    
    REFERENCE_TIME requestedBufferDuration = REF_TIMES_PER_SECOND * 2;
    
    result = audio.audioClient->lpVtbl->Initialize(
                                                   audio.audioClient, AUDCLNT_SHAREMODE_SHARED, 
                                                   AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | 
                                                   AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, 
                                                   requestedBufferDuration, 0, &waveFormat, 0);
    
    if(result != S_OK)
    {
        switch(result)
        {
            case AUDCLNT_E_WRONG_ENDPOINT_TYPE:{ LogError("AUDCLNT_E_WRONG_ENDPOINT_TYPE");break;}
            case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED:{ LogError("AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED");break;}
            case AUDCLNT_E_BUFFER_SIZE_ERROR:{ LogError("AUDCLNT_E_BUFFER_SIZE_ERROR");break;}
            case AUDCLNT_E_CPUUSAGE_EXCEEDED:{ LogError("AUDCLNT_E_CPUUSAGE_EXCEEDED");break;}
            case AUDCLNT_E_DEVICE_INVALIDATED:{ LogError("AUDCLNT_E_DEVICE_INVALIDATED");break;}
            case AUDCLNT_E_DEVICE_IN_USE:{ LogError("AUDCLNT_E_DEVICE_IN_USE");break;}
            case AUDCLNT_E_ENDPOINT_CREATE_FAILED:{ LogError("AUDCLNT_E_ENDPOINT_CREATE_FAILED");break;}
            case AUDCLNT_E_INVALID_DEVICE_PERIOD:{ LogError("AUDCLNT_E_INVALID_DEVICE_PERIOD");break;}
            case AUDCLNT_E_UNSUPPORTED_FORMAT:{ LogError("AUDCLNT_E_UNSUPPORTED_FORMAT");break;}
            case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:{ LogError("AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED");break;}
            case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL:{ LogError("AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL");break;}
            case AUDCLNT_E_SERVICE_NOT_RUNNING:{ LogError("AUDCLNT_E_SERVICE_NOT_RUNNING");break;}
            case E_POINTER:{ LogError("E_POINTER");break;}
            case E_INVALIDARG:{ LogError("E_INVALIDARG");break;}
            case E_OUTOFMEMORY:{ LogError("E_OUTOFMEMORY");break;}
        }
        LogError("IAudioClient Initialize");
        return audio;
    }
    
    audio.audioRenderClient;
    result = audio.audioClient->lpVtbl->GetService(
                                                   audio.audioClient, &IID_IAudioRenderClient, (void**)&audio.audioRenderClient 
                                                   );
    
    if(result != S_OK)
    {
        LogError("IAudioClient GetService");
        return audio;
    }
    
    audio.audioClient->lpVtbl->GetBufferSize(audio.audioClient, &audio.bufferFrameCount);
    
    audio.bufferDuration = (REFERENCE_TIME)((f64)REF_TIMES_PER_SECOND *
                                            (audio.bufferFrameCount / audio.samplesPerSecond));
    
    LogInfo("WASAPI Audio buffer frame count: %d", audio.bufferFrameCount);
    LogInfo("WASAPI Audio buffer duration: %lld (10000000 is 1 second)", audio.bufferDuration);
    
    result = audio.audioClient->lpVtbl->Start(audio.audioClient);
    if(result != S_OK)
    {
        LogError("IAudioClient Start");
        return audio;
    };
    
    audio.initialized = 1;
    LogSuccess("WASAPI Initialized");
    
    return audio;
}

internal void
Win32AudioBufferFill(u32 samplesToWrite, i16 *samples, win32_audio_data *output)
{
    if(samplesToWrite)
    {
        BYTE *data = 0;
        DWORD flags = 0;
        
        output->audioRenderClient->lpVtbl->GetBuffer(
                                                     output->audioRenderClient, samplesToWrite, &data
                                                     );
        if(data)
        {
            i16 *destination = (i16 *)data;
            i16 *source = samples;
            for(UINT32 i = 0; i < samplesToWrite; ++i)
            {
                i16 left = (i16)(*source++);
                i16 right = (i16)(*source++);
                *destination++ = left;
                *destination++ = right;
            }
        }
        output->audioRenderClient->lpVtbl->ReleaseBuffer(
                                                         output->audioRenderClient, samplesToWrite, flags
                                                         );
    }
}

internal u32
Win32AudioStatusUpdate(win32_audio_data *audioData, f32 currentFramesPerSecond, f32 latencyMultiplier)
{
    u32 samplesToWrite = 0;
    if(audioData->initialized)
    {
        audioData->latencyFrameCount = (u32)(audioData->samplesPerSecond / 
                                             currentFramesPerSecond * 
                                             latencyMultiplier);
        UINT32 padding;
        if(SUCCEEDED(audioData->audioClient->lpVtbl->GetCurrentPadding(audioData->audioClient, &padding)))
        {
            samplesToWrite = audioData->latencyFrameCount - padding;
            if(samplesToWrite > audioData->latencyFrameCount)
            {
                samplesToWrite = audioData->latencyFrameCount;
            }
        }
    }
    
    return samplesToWrite;
}

internal void
CleanAudioBuffer(void *audioBuffer, u32 audioBufferSize)
{
    memset(audioBuffer, 0, audioBufferSize);
}

internal void
Win32WasapiCleanup(win32_audio_data *audio)
{
    if(audio->initialized)
    {
        audio->audioClient->lpVtbl->Stop(audio->audioClient);
        
        audio->deviceEnum->lpVtbl->Release(audio->deviceEnum);
        audio->device->lpVtbl->Release(audio->device);
        audio->audioClient->lpVtbl->Release(audio->audioClient);
        audio->audioRenderClient->lpVtbl->Release(audio->audioRenderClient);
    }
}

internal iv2
Win32WindowGetSize()
{
    RECT ClientRect;
    iv2 windowSize;
    
    // NOTE: get size of the window, without the border
    GetWindowRect(GLOBALWindow, &ClientRect);
    windowSize.width = ClientRect.right - ClientRect.left;
    windowSize.height = ClientRect.bottom - ClientRect.top;
    
    return windowSize;
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
            // NOTE: resize opengl viewport on window resize
            if(GLOBALRenderer == RENDERER_OPENGL)
            {
                Win32OpenGLAspectRatioUpdate(16, 9);
            }
            break;
        }
        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            Assert(0);
        }
        
        case WM_PAINT:
        {
            if(GLOBALRenderer == RENDERER_SOFTWARE)
            {
                PAINTSTRUCT paint;
                HDC DeviceContext = BeginPaint(window, &paint);
                iv2 windowSize = Win32WindowDrawAreaGetSize();
                Win32DisplayBufferInWindow(&GLOBALGraphicsBuffer, DeviceContext,
                                           windowSize.x, windowSize.y);
                EndPaint(window, &paint);
            }
            else
            {
                result = DefWindowProc(window, message, wParam, lParam);
            }
        } break;
        
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

internal f32
MonitorGetRefreshRate()
{
    return GLOBALMonitorRefreshRate;
}

internal bool32
VSyncGetState()
{
    return GLOBALVSyncState;
}

internal u64
TimeGetProcessorCycles()
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

internal iv2
WindowBorderGetSize()
{
    iv2 windowDrawAreaSize = Win32WindowDrawAreaGetSize();
    iv2 windowSize = Win32WindowGetSize();
    
    iv2 borderSize;
    // NOTE: Calculate how big is the border
    borderSize.width = windowSize.width - windowDrawAreaSize.width;
    borderSize.height = windowSize.height - windowDrawAreaSize.height;
    
    return borderSize;
}


// Sets the size of the draw area
internal void
WindowDrawAreaSetSize(i32 width, i32 height)
{
    iv2 borderSize = WindowBorderGetSize();
    
    // NOTE: Add border size to the total width and height
    i32 actualWidth = width + borderSize.width;
    i32 actualHeight = height + borderSize.height;
    
    bool32 result = SetWindowPos(GLOBALWindow, 0, 
                                 0, 0, actualWidth, actualHeight, 
                                 SWP_NOMOVE | SWP_NOOWNERZORDER);
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
WindowReloadAttributes()
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
WindowDrawBorder(bool32 draw)
{
    static iv2 GLOBALWindowBorderSize;
    static bool32 GLOBALIsBorderDrawn;
    
    if(GLOBALWindowBorderSize.width == 0 && GLOBALWindowBorderSize.height == 0)
        GLOBALWindowBorderSize = WindowBorderGetSize();
    
    if(draw)
    {
        if(!SetWindowLongA(GLOBALWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE))
        {
            LogError("");
        }
        if(!GLOBALIsBorderDrawn)
        {
            iv2 windowSize = Win32WindowGetSize();
            WindowSetSize(windowSize.width + GLOBALWindowBorderSize.width,
                          windowSize.height + GLOBALWindowBorderSize.height);
        } 
        WindowReloadAttributes();
        GLOBALIsBorderDrawn = 1;
    }
    else
    {
        iv2 drawAreaSize = Win32WindowDrawAreaGetSize();
        WindowSetSize(drawAreaSize.width, drawAreaSize.height);
        if(!SetWindowLongA(GLOBALWindow, GWL_STYLE, WS_VISIBLE))
        {
            LogError("");
        }
        WindowReloadAttributes();
        GLOBALIsBorderDrawn = 0;
    }
}

internal void
ScreenDraw(HDC deviceContext)
{
    if(GLOBALRenderer == RENDERER_SOFTWARE)
    {
        iv2 windowSize = Win32WindowDrawAreaGetSize();
        Win32DisplayBufferInWindow(&GLOBALGraphicsBuffer, deviceContext,
                                   windowSize.x, windowSize.y);
    }
    else if(GLOBALRenderer == RENDERER_OPENGL)
    {
        wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE);
    }
}

int 
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, i32 showCode)
{
    // NOTE: Attach to the console that invoked the app
    Win32ConsoleAttach();
    GLOBALRenderer = RENDERER_START;
    
    // NOTE: Init time data
    
    // NOTE: Set windows scheduler to wake up every 1 millisecond so
    //       so that the Sleep function will work properly for our purposes
    GLOBALSleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
    GLOBALCountsPerSecond = Win32PerformanceFrequencyGet();
    
    // NOTE: Set timers to application start
    u64 startAppCycles = ProcessorClockCycles();
    i64 startAppCount = Win32PerformanceCountGet();
    f32 startAppMilliseconds = PerformanceCountToMilliseconds(startAppCount);
    
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
                                   WINDOW_TITLE, 
                                   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                   DEFAULT_WINDOW_POS_X, 
                                   DEFAULT_WINDOW_POS_Y, 
                                   CW_USEDEFAULT, 
                                   CW_USEDEFAULT, 
                                   0, 0, instance, 0);
    
    if(!GLOBALWindow) {LogError("Create Window"); return 0;}
    
    // NOTE: 0 - 255 (not transparent at all)
    WindowSetTransparency(WINDOW_TRANSPARENCY);
    
    // NOTE: Set the draw area size to be equal to specified window size
    WindowDrawAreaSetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // NOTE: Window context setup
    HDC deviceContext = GetDC(GLOBALWindow);
    
    iv2 windowSize = Win32WindowGetSize();
    iv2 drawAreaSize = Win32WindowDrawAreaGetSize();
    LogInfo("Window size %d %d", windowSize.width, windowSize.height);
    LogInfo("Window draw area size %d %d", drawAreaSize.width, drawAreaSize.height);
    
    // NOTE(KKrzosa): SETUP RENDERER
    if(GLOBALRenderer == RENDERER_OPENGL)
    {
        // NOTE: Setup openGL context, choose pixel format, load wgl functions
        //       function for setting vsync is loaded here
        HGLRC openglContext = Win32OpenGLInit(deviceContext);
        // NOTE: Set the opengl viewport to match the aspect ratio
        //Win32OpenGLAspectRatioUpdate(16, 9);
        LogSuccess("OPENGL VERSION: %s", glGetString(GL_VERSION));
    }
    else if(GLOBALRenderer == RENDERER_SOFTWARE)
    {
        Win32ResizeDIBSection(&GLOBALGraphicsBuffer, drawAreaSize.x, drawAreaSize.y);
    }
    
    // NOTE: Construct paths to exe and to dll
    str8 *pathToExeDirectory = Win32GetExecutableDirectory();
    str8 *mainDLLPath = StringConcatChar(pathToExeDirectory, "/app_code.dll");
    str8 *tempDLLPath = StringConcatChar(pathToExeDirectory, "/app_code_temp.dll");
    LogInfo("Paths\n PathToExeDirectory: %s \n PathToDLL %s \n PathToTempDLL %s", 
            pathToExeDirectory, mainDLLPath, tempDLLPath);
    
    // NOTE: Load the dll and call initialize function
    win32_dll_code dllCode = {0};
    dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);
    
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
    operating_system_interface os = {0};
    {
        os.pernamentStorage.maxSize = Megabytes(64);
        os.temporaryStorage.maxSize = Megabytes(64);
        os.audioBufferSize = AudioBufferSize(audioData);
        
        os.pernamentStorage.memory = VirtualAlloc(0, 
                                                  os.pernamentStorage.maxSize + 
                                                  os.temporaryStorage.maxSize + 
                                                  os.audioBufferSize, 
                                                  MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
                                                  );
        
        os.temporaryStorage.memory = ((i8 *)os.pernamentStorage.memory + os.pernamentStorage.maxSize);
        os.audioBuffer = (i8 *)os.temporaryStorage.memory + os.temporaryStorage.maxSize;
        
        LogSuccess("OS Memory allocated");
        
        os.pathToExecutableDirectory = pathToExeDirectory;
        
        os.audioLatencyMultiplier = 4.f;
        os.samplesPerSecond = audioData.samplesPerSecond;
        os.targetFramesPerSecond = GLOBALMonitorRefreshRate;
        
        os.timeData.startAppCount = startAppCount;
        os.timeData.startAppCycles = startAppCycles;
        os.timeData.startAppMilliseconds = startAppMilliseconds;
        os.timeData.countsPerSecond = GLOBALCountsPerSecond;
        
        os.currentRenderer = GLOBALRenderer;
        
        os.Log                                 = &ConsoleLog;
        os.LogExtra                            = &ConsoleLogExtra;
        os.Quit                                = &Quit;
        
        os.FileRead                            = &Win32FileRead;
        os.FileGetSize                         = &Win32FileGetSize;
        os.DirectoryReadAllFiles               = &Win32DirectoryReadAllFiles;
        
        os.TimeGetMilliseconds                 = &Win32MillisecondsGet;
        os.TimeGetCounts                       = &Win32PerformanceCountGet;
        os.TimeGetProcessorCycles              = &TimeGetProcessorCycles;
        
        os.VSyncSetState                       = &Win32OpenGLSetVSync;
        os.VSyncGetState                       = &VSyncGetState;
        os.MonitorGetRefreshRate               = &MonitorGetRefreshRate;
        
        os.WindowSetTransparency               = &WindowSetTransparency;
        os.WindowAlwaysOnTop                   = &WindowAlwaysOnTop;
        os.WindowNotAlwaysOnTop                = &WindowNotAlwaysOnTop;
        os.WindowGetSize                       = &Win32WindowDrawAreaGetSize;
        os.WindowSetSize                       = &WindowDrawAreaSetSize;
        os.WindowSetPosition                   = &WindowSetPosition;
        os.WindowDrawBorder                    = &WindowDrawBorder;
        
        os.OpenGLLoadProcedures                = &Win32OpenGLLoadProcedures;
        
        LogSuccess("OS Functions Loaded");
    }
    
    dllCode.initialize(&os);
    
    
    // NOTE: Begin timming the frame
    u64 beginFrameCycles = ProcessorClockCycles();
    i64 beginFrame = Win32PerformanceCountGet();
    
    GLOBALApplicationIsRunning = true;
    while(GLOBALApplicationIsRunning)
    {
        Win32UpdateDLLCode(&dllCode, mainDLLPath, tempDLLPath, &os);
        
        // NOTE: Process input, keyboard and mouse
        Win32InputUpdate(&os.userInput);
        // NOTE: Process input, controller
        Win32XInputUpdate(&os.userInput);
        
        // NOTE: Figure out how much sound to write and where / update the latency based on
        // potential fps changes
        u32 samplesToWrite = Win32AudioStatusUpdate(&audioData, 
                                                    os.targetFramesPerSecond,
                                                    os.audioLatencyMultiplier);
        // TODO: should it maybe be clamped to samplesToWrite?
        // CleanAudioBuffer(os.audioBuffer, os.audioBufferSize);
        
        // NOTE: Update operating system status
        {
            os.requestedSamples = samplesToWrite;
            os.samplesPerSecond = audioData.samplesPerSecond;
            
            if(os.currentRenderer == RENDERER_SOFTWARE)
            {
                os.graphicsBuffer.memory = GLOBALGraphicsBuffer.memory;
                os.graphicsBuffer.size.x = GLOBALGraphicsBuffer.width;
                os.graphicsBuffer.size.y = GLOBALGraphicsBuffer.height;
                os.graphicsBuffer.pitch = GLOBALGraphicsBuffer.pitch;
            }
            // NOTE: Call Update function from the dll, bit "and" operator here
            //       because we dont want update to override appstatus
            dllCode.update(&os);
        }
        
        if(audioData.initialized)
        {
            Win32AudioBufferFill(samplesToWrite, os.audioBuffer, &audioData);
        }
        
        f32 msPerFrame = (1 / os.targetFramesPerSecond * 1000);
        EndFrameAndSleep(&os.timeData, msPerFrame, &beginFrame, &beginFrameCycles);
        
        ScreenDraw(deviceContext);
    }
    
    return(1);
}

#endif
