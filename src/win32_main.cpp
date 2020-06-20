#include <windows.h>
#include <stdint.h>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>
#include "win32_main.h"
#include "render.h"
#include "render.cpp"

global_variable bool GlobalRunning = true;
global_variable user_input GlobalUserInput;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global_variable x_input_get_state *XInputGetStateFunction = XInputGetStateStub;
global_variable x_input_set_state *XInputSetStateFunction = XInputSetStateStub;
#define Pi32 3.14159265359f

#include "audio.cpp"

struct file
{
    void *contents;
    u32 size;
};

internal void
FreeFileMemory(void *Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

internal file 
ReadEntireFile(char *filename)
{
    file result = {};
    
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(fileHandle, &FileSize))
        {
            u32 fileSize32 = (u32)FileSize.QuadPart;
            result.contents = VirtualAlloc(0, fileSize32 + 1, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(result.contents)
            {
                DWORD BytesRead;
                if(ReadFile(fileHandle, result.contents, fileSize32, &BytesRead, 0) && (fileSize32 == BytesRead))
                {
                    result.size = fileSize32;
                    OutputDebugStringA("ReadEntireFile: Success\n");
                }
                else
                {                    
                    OutputDebugStringA("ReadEntireFile: ReadFile failed\n");
                    FreeFileMemory(result.contents);
                    result.contents = 0;
                }
            }
            else
            {
                OutputDebugStringA("ReadEntireFile: NULL file contents\n");
            }
        }
        else
        {
            OutputDebugStringA("ReadEntireFile: Failed to get file size\n");
        }

        CloseHandle(fileHandle);
    }
    else
    {
        OutputDebugStringA("ReadEntireFile: Invalid file handle name\n");
    }

    return(result);
}

internal void 
Win32LoadXInput(void)    
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");

    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
        OutputDebugStringA("xinput1_4.dll Failed");
    }

    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
        OutputDebugStringA("xinput9_1_0.dll Failed");
    }

    if(XInputLibrary)
    {
        XInputGetStateFunction = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetStateFunction) {XInputGetStateFunction = XInputGetStateStub;}

        XInputSetStateFunction = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetStateFunction) {XInputSetStateFunction = XInputSetStateStub;}

        // TODO:
    }
    else
    {
        // TODO: 
    }
}

window_dimension 
Win32GetWindowDimension(HWND Window)
{
    RECT ClientRect;
    window_dimension windowDimension;
    // get size of the window, without the border
    GetClientRect(Window, &ClientRect);
    windowDimension.width = ClientRect.right - ClientRect.left;
    windowDimension.height = ClientRect.bottom - ClientRect.top;
    return windowDimension;
}

internal void 
Win32ResizeDIBSection(win32_offscreen_buffer* buffer, i32 width, i32 height)
{
    // if we dont free before allocating, memory will leak
    if(buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    i32 bytesPerPixel = 4;
    buffer->width = width;
    buffer->height = height;
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB; //uncompressed RGB

    
    i32 bitmapMemorySize = bytesPerPixel * (buffer->width * buffer->height);
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = width * bytesPerPixel;
}

internal void 
Win32DrawBufferToScreen(HDC DeviceContext, i32 windowWidth, i32 windowHeight, win32_offscreen_buffer* buffer)
{
    // The StretchDIBits function copies the color data for a rectangle of pixels in a DIB, 
    // to the specified destination rectangle. 
    // If the destination rectangle is larger than the source rectangle, 
    // this function stretches the rows and columns of color data to fit the destination rectangle. 
    // If the destination rectangle is smaller than the source rectangle, 
    // this function compresses the rows and columns by using the specified raster operation.
    StretchDIBits(
        DeviceContext,
        0, 0, windowWidth, windowHeight,
        0, 0, buffer->width, buffer->height,
        buffer->memory,
        &buffer->info,
        DIB_RGB_COLORS, SRCCOPY);
}

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size, 0)))
    {
        DWORD Region1SampleCount = Region1Size/SoundOutput->bytesPerSample;
        i16 *SampleOut = (i16 *)Region1;
        for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
        {
            float SineValue = sinf(SoundOutput->tSine);
            i16 SampleValue = (i16)(SineValue * SoundOutput->toneVolume);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;

            SoundOutput->tSine += 2.0f*Pi32*1.0f/(float)SoundOutput->wavePeriod;
            ++SoundOutput->runningSampleIndex;
        }

        DWORD Region2SampleCount = Region2Size/SoundOutput->bytesPerSample;
        SampleOut = (i16 *)Region2;
        for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
        {
            float SineValue = sinf(SoundOutput->tSine);
            i16 SampleValue = (i16)(SineValue * SoundOutput->toneVolume);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;

            SoundOutput->tSine += 2.0f*Pi32*1.0f/(float)SoundOutput->wavePeriod;
            ++SoundOutput->runningSampleIndex;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}


LRESULT CALLBACK 
Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    uint32_t VKCode = WParam;
    switch (Message) 
    {
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            GlobalRunning = false;
            break;
        } 
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
            break;
        } 
        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
            GlobalRunning = false;
            break;
        } 
        case WM_QUIT:
        {
            GlobalRunning = false;
            break;
        }

        case WM_KEYDOWN:
        {
            if(VKCode == 'W')
            {
                GlobalUserInput.up = 1;
            }
            else if(VKCode == 'S')
            {
                GlobalUserInput.down = 1;
            }
            if(VKCode == 'A')
            {
                GlobalUserInput.left = 1;
            }
            else if(VKCode == 'D')
            {
                GlobalUserInput.right = 1;
            }
            else if(VKCode == VK_ESCAPE)
            {
                GlobalRunning = 0;
            }
            else if(VKCode == VK_F1)
            {
                GlobalUserInput.reset = 1;
            }
            break;
        }
        case WM_KEYUP:
        {
            if(VKCode == 'W')
            {
                GlobalUserInput.up = 0;
            }
            else if(VKCode == 'S')
            {
                GlobalUserInput.down = 0;
            }
            if(VKCode == 'A')
            {
                GlobalUserInput.left = 0;
            }
            else if(VKCode == 'D')
            {
                GlobalUserInput.right = 0;
            }
            else if(VKCode == VK_F1)
            {
                GlobalUserInput.reset = 0;
            }
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            window_dimension dimension = Win32GetWindowDimension(Window);
            Win32DrawBufferToScreen(DeviceContext, 
                                       dimension.width, dimension.height, 
                                       &GlobalBackbuffer);
            EndPaint(Window, &Paint);
        } break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
        
    }

    return Result;
}


i32 CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, i32 ShowCode)
{
    Win32LoadXInput();
    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    WNDCLASSA WindowClass = {};
    {
        WindowClass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
        WindowClass.lpfnWndProc    = Win32MainWindowCallback;
        WindowClass.cbClsExtra     = 0;
        WindowClass.cbWndExtra     = 0;
        WindowClass.hInstance      = Instance;
        WindowClass.hIcon          = LoadIcon(Instance, IDI_APPLICATION);
        WindowClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
        WindowClass.lpszMenuName   = NULL;
        WindowClass.lpszClassName  = ("PLACEHOLDER");
    }
    
    if (!RegisterClassA(&WindowClass))
    {
        OutputDebugStringA("FAILED to register WindowClass\n");
    }

    HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, 
        "TITLE_PLACEHOLDER", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, 
        CW_USEDEFAULT, NULL, NULL, Instance, NULL);

    if(!Window)
    {
        OutputDebugStringA("FAILED to create a Window\n");
    }

    HDC DeviceContext = GetDC(Window);

    win32_sound_output SoundOutput = {};
    {
        SoundOutput.samplesPerSecond = 48000;
        SoundOutput.toneHz = 256;
        SoundOutput.toneVolume = 1500;
        SoundOutput.wavePeriod = SoundOutput.samplesPerSecond/SoundOutput.toneHz;
        SoundOutput.bytesPerSample = sizeof(i16)*2;
        SoundOutput.secondaryBufferSize = SoundOutput.samplesPerSecond*SoundOutput.bytesPerSample;
        SoundOutput.latencySampleCount = SoundOutput.samplesPerSecond / 15;
    }

    Win32InitDSound(Window, SoundOutput.samplesPerSecond, SoundOutput.secondaryBufferSize);
    Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.latencySampleCount*SoundOutput.bytesPerSample);
    // GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
    
    file objFile = ReadEntireFile("obj.obj");
    char *fileContents = (char *)objFile.contents;

    v2 offset = {0, 0};
    i32 musicIndex = 1;
    while(GlobalRunning)
    {
        MSG Message;
        while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }

        DWORD xinputState;    
        // i = controller index
        for (DWORD i=0; i < XUSER_MAX_COUNT; i++ )
        {

            XINPUT_STATE state;
            if(XInputGetStateFunction( i, &state ) == ERROR_SUCCESS)
            {
                XINPUT_GAMEPAD *gamepad = &state.Gamepad;
                bool up = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                bool down = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                bool left = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                bool right = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                bool start = (gamepad->wButtons & XINPUT_GAMEPAD_START);
                bool back = (gamepad->wButtons & XINPUT_GAMEPAD_BACK);
                bool leftShoulder = (gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                bool rightShoulder = (gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                bool AButton = (gamepad->wButtons & XINPUT_GAMEPAD_A);
                bool BButton = (gamepad->wButtons & XINPUT_GAMEPAD_B);
                bool XButton = (gamepad->wButtons & XINPUT_GAMEPAD_X);
                bool YButton = (gamepad->wButtons & XINPUT_GAMEPAD_Y);

                i16 stickX = gamepad->sThumbLX;
                i16 stickY = gamepad->sThumbLY;

                offset.x += stickX / 4096 / 2;
                offset.y -= stickY / 4096 / 2;

                // SoundOutput.toneHz = 512 + (int)(256.0f*((float)stickY / 30000.0f));
                // SoundOutput.wavePeriod = SoundOutput.samplesPerSecond/SoundOutput.toneHz;
            }
            else
            {
                OutputDebugStringA("controller not connected\n");
            }
        }

        offset.y += GlobalUserInput.up;
        offset.y -= GlobalUserInput.down;
        offset.x += GlobalUserInput.right;
        offset.x -= GlobalUserInput.left;
        v4 lineColor = {0,255,255,255};
        if(GlobalUserInput.reset)
        {
            DrawRectangle(&GlobalBackbuffer, 0, 0, 1280, 720, 0);
        }

        // DrawGradient(&GlobalBackbuffer, offsetX, offsetY);
        // DrawLineFirst(&GlobalBackbuffer, {100, 200}, {500, 600}, lineColor);
        // DrawLineSecond(&GlobalBackbuffer, {200, 200}, {600, 600}, lineColor);
        DrawLineFinal(&GlobalBackbuffer, 10, 100, 500 + offset.x, 500 + offset.y, {0,200,100});

        DWORD PlayCursor;
        DWORD WriteCursor;
        #if 0
        if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
        {
            DWORD ByteToLock = ((SoundOutput.runningSampleIndex*SoundOutput.bytesPerSample) %
                                SoundOutput.secondaryBufferSize);

            DWORD TargetCursor = ((PlayCursor + 
                (SoundOutput.latencySampleCount*SoundOutput.bytesPerSample)) %
                 SoundOutput.secondaryBufferSize);

            DWORD BytesToWrite;

            if(ByteToLock > TargetCursor)
            {
                BytesToWrite = (SoundOutput.secondaryBufferSize - ByteToLock);
                BytesToWrite += TargetCursor;
            }
            else
            {
                BytesToWrite = TargetCursor - ByteToLock;
            }

            Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite);
        }
        #endif
        
        window_dimension dimension = Win32GetWindowDimension(Window);
        Win32DrawBufferToScreen(DeviceContext, 
                                   dimension.width, dimension.height, 
                                   &GlobalBackbuffer);
    }
    
    
    
    return(0);
}