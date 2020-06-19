#include <windows.h>
#include <stdint.h>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>
#include "render.cpp"
#include "win32_main.h"

global_variable bool GlobalRunning = true;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

#include "audio.cpp"

#define Pi32 3.14159265359f

static int offsetX = 0;
static int offsetY = 0;

global_variable x_input_get_state *XInputGetStateFunction = XInputGetStateStub;
global_variable x_input_set_state *XInputSetStateFunction = XInputSetStateStub;

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
Win32ResizeDIBSection(win32_offscreen_buffer* buffer, 
                                    int width, int height)
{
    // if we dont free before allocating, memory will leak
    if(buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;
    int bytesPerPixel = 4;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    // When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB; //uncompressed RGB

    
    int bitmapMemorySize = bytesPerPixel * (buffer->width * buffer->height);
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = width * bytesPerPixel;
}

internal void 
Win32DrawBufferToScreen(HDC DeviceContext, int windowWidth, int windowHeight, win32_offscreen_buffer* buffer)
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
                                             &Region2, &Region2Size,
                                             0)))
    {
        DWORD Region1SampleCount = Region1Size/SoundOutput->bytesPerSample;
        int16_t *SampleOut = (int16_t *)Region1;
        for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
        {
            float SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * SoundOutput->toneVolume);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;

            SoundOutput->tSine += 2.0f*Pi32*1.0f/(float)SoundOutput->wavePeriod;
            ++SoundOutput->runningSampleIndex;
        }

        DWORD Region2SampleCount = Region2Size/SoundOutput->bytesPerSample;
        SampleOut = (int16_t *)Region2;
        for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
        {
            float SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * SoundOutput->toneVolume);
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

    switch (Message) 
    {
        // WM_NCPAINT message - to draw on frame / titlebar
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            GlobalRunning = false;
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
            GlobalRunning = false;
        } break;
        case WM_QUIT:
        {
            GlobalRunning = false;
        }

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            uint32_t VKCode = WParam;
            bool wasDown = ((LParam & (1 << 30)) != 0);
            bool isDown  = ((LParam & (1 << 31)) == 0);
            if (wasDown != isDown)
            {
                if(VKCode == 'W')
                {
                    OutputDebugStringA("W: ");
                    if(isDown)
                    {
                        OutputDebugStringA("IsDown ");
                        offsetY-=5;
                    }
                    if(wasDown)
                    {
                        OutputDebugStringA("WasDown");
                    }

                    OutputDebugStringA("\n");
                }
                else if(VKCode == 'S')
                {
                    offsetY+=5;
                }
                else if(VKCode == 'A')
                {
                    offsetX-=5;
                }
                else if(VKCode == 'D')
                {
                    offsetX+=5;
                }
                else if(VKCode == VK_UP)
                {
                }
                else if(VKCode == VK_DOWN)
                {
                }
                else if(VKCode == VK_LEFT)
                {
                }
                else if(VKCode == VK_RIGHT)
                {
                }
                else if(VKCode == VK_ESCAPE)
                {
                    GlobalRunning = false; // DEBUG
                }
                
            }
            bool AltKeyWasDown = (LParam & (1 << 29));
            if((VKCode == VK_F4) && AltKeyWasDown)
            {
                GlobalRunning = false;
            }
        } break;

        case WM_PAINT:
        {
            // The PAINTSTRUCT structure contains information for an application. 
            // This information can be used to paint the client area of a window owned by that application.
            PAINTSTRUCT Paint;
            // The BeginPaint function prepares the specified window for 
            // painting and fills a PAINTSTRUCT structure with information about the painting.
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


int CALLBACK 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
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
        SoundOutput.bytesPerSample = sizeof(int16_t)*2;
        SoundOutput.secondaryBufferSize = SoundOutput.samplesPerSecond*SoundOutput.bytesPerSample;
        SoundOutput.latencySampleCount = SoundOutput.samplesPerSecond / 15;
    }

    Win32InitDSound(Window, SoundOutput.samplesPerSecond, SoundOutput.secondaryBufferSize);
    Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.latencySampleCount*SoundOutput.bytesPerSample);
    GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
    
    int musicIndex = 1;
    while(GlobalRunning)
    {
        MSG Message;
        // GetMessage does not return until a message come in
        // PeekMessage returns regardless whether message is in the queue
        // Better for games
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

                int16_t stickX = gamepad->sThumbLX;
                int16_t stickY = gamepad->sThumbLY;

                offsetX += stickX / 4096 / 2;
                offsetY -= stickY / 4096 / 2;

                SoundOutput.toneHz = 512 + (int)(256.0f*((float)stickY / 30000.0f));
                SoundOutput.wavePeriod = SoundOutput.samplesPerSecond/SoundOutput.toneHz;
            }
            else
            {
                OutputDebugStringA("controller not connected\n");
            }
        }


        RenderWeirdGradient(&GlobalBackbuffer, offsetX, offsetY);
        // RenderRectangle(&GlobalBackbuffer, 0, 0, 1280, 720, 0);
        // RenderRectangle(&GlobalBackbuffer, 100+offsetX, 100+offsetY, 100, 100, 255);


        DWORD PlayCursor;
        DWORD WriteCursor;
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
        
        window_dimension dimension = Win32GetWindowDimension(Window);
        Win32DrawBufferToScreen(DeviceContext, 
                                   dimension.width, dimension.height, 
                                   &GlobalBackbuffer);
    }
    
    
    
    return(0);
}