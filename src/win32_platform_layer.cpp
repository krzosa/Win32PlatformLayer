#include <windows.h>
#include <stdint.h>
#include <Xinput.h>
#include <stdio.h>

#include "win32_platform_layer.h"

// loop variable
global_variable bool GlobalRunning = true;
global_variable win32_offscreen_buffer GlobalBackbuffer;

int clamp(int min, int value, int max)
{
    if(value > max) return max;
    else if(value < min) return min;
    return value;
}

internal void RenderRectangle(win32_offscreen_buffer buffer, int x, int y, int width, int height)
{
    uint8_t *Row = (uint8_t *)buffer.memory; 
    Row = Row + (y * buffer.pitch);
    for(int Y = 0; Y < height; Y++)
    {
        uint32_t *Pixel = (uint32_t*)Row + x;
        for(int X = 0; X < width; X++)
        {
            uint8_t red = 50;
            uint8_t green = 180;
            uint8_t blue = 100;

            *Pixel = ((red << 16) | (green << 8) | (blue));
            Pixel = Pixel + 1;
        }
        Row = Row + buffer.pitch;
    }
}

internal void RenderWeirdGradient(win32_offscreen_buffer buffer, int BlueOffset, int GreenOffset)
{    
    uint8_t *Row = (uint8_t *)buffer.memory;    
    for(int Y = 0; Y < buffer.height; ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for(int X = 0; X < buffer.width; ++X)
        {
            uint8_t Blue = (X + BlueOffset);
            uint8_t Green = (Y + GreenOffset);
            
            *Pixel++ = ((Green << 16) | (Blue << 8) );
        }

        Row += buffer.pitch;
    }
}

window_dimension Win32GetWindowDimension(HWND Window)
{
    RECT ClientRect;
    window_dimension windowDimension;
    // get size of the window, without the border
    GetClientRect(Window, &ClientRect);
    windowDimension.width = ClientRect.right - ClientRect.left;
    windowDimension.height = ClientRect.bottom - ClientRect.top;
    return windowDimension;
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer* buffer, 
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
    // color bits
    buffer->info.bmiHeader.biBitCount = 32;
    // RGB = uncompressed
    buffer->info.bmiHeader.biCompression = BI_RGB;

    
    int bitmapMemorySize = bytesPerPixel * (buffer->width * buffer->height);
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = width * bytesPerPixel;
}

internal void Win32DisplayBufferInWindow(HDC DeviceContext, 
                                        int windowWidth, int windowHeight, 
                                        win32_offscreen_buffer buffer)
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
        0, 0, buffer.width, buffer.height,
        buffer.memory,
        &buffer.info,
        DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK Win32MainWindowCallback(HWND   Window,
                                         UINT   Message,
                                         WPARAM WParam,
                                         LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message) 
    {
        // WM_NCPAINT message - to draw on frame / titlebar
        // case WM_EXITSIZEMOVE:
        // {
        //     window_dimension dimension = Win32GetWindowDimension(Window);
        //     Win32ResizeDIBSection(&GlobalBackbuffer, dimension.width, dimension.height);
        //     OutputDebugStringA("WM_SIZE\n");
        // } break;
        // WM_CLOSE is called when user sends signal to terminate the application
        // we can handle the closing procedure here
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            GlobalRunning = false;
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        // WM_DESTROY is sent when a window is being destroyed. 
        // It is sent to the window procedure of the 
        // window being destroyed after the window is removed from the screen.
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
                    }
                    if(wasDown)
                    {
                        OutputDebugStringA("WasDown");
                    }
                    OutputDebugStringA("\n");
                }
                else if(VKCode == 'S')
                {
                }
                else if(VKCode == 'A')
                {
                }
                else if(VKCode == 'D')
                {
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
        }

        case WM_PAINT:
        {
            // The PAINTSTRUCT structure contains information for an application. 
            // This information can be used to paint the client area of a window owned by that application.
            PAINTSTRUCT Paint;
            // The BeginPaint function prepares the specified window for 
            // painting and fills a PAINTSTRUCT structure with information about the painting.
            HDC DeviceContext = BeginPaint(Window, &Paint);
            window_dimension dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(DeviceContext, 
                                       dimension.width, dimension.height, 
                                       GlobalBackbuffer);
            EndPaint(Window, &Paint);
        } break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
        
    }

    return Result;
}


int CALLBACK WinMain(  HINSTANCE Instance,
             HINSTANCE PrevInstance,
             LPSTR     CommandLine,
             int       ShowCode)
{
    WNDCLASSA WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    WindowClass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
    WindowClass.lpfnWndProc    = Win32MainWindowCallback;
    WindowClass.cbClsExtra     = 0;
    WindowClass.cbWndExtra     = 0;
    WindowClass.hInstance      = Instance;
    WindowClass.hIcon          = LoadIcon(Instance, IDI_APPLICATION);
    WindowClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    WindowClass.lpszMenuName   = NULL;
    WindowClass.lpszClassName  = ("PLACEHOLDER");
    
    if (RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(0,
            WindowClass.lpszClassName,
            "TITLE_PLACEHOLDER",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, Instance, NULL);
        if(Window)
        {

            HDC DeviceContext = GetDC(Window);
            int offsetX = 0;
            int offsetY = 0;

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
                for (DWORD i=0; i < 4; i++ )
                {

                    XINPUT_STATE state;
                    if(XInputGetState( i, &state ) == ERROR_SUCCESS)
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

                        if (up == 1)
                        {
                            OutputDebugStringA("DPAD UP IS PRESSED\n");
                        }
                        // OutputDebugStringA("CONTROLLER CONECTED\n");
                    }
                    else
                    {
                        // OutputDebugStringA("controller not connected\n");
                    }
                }


                // RenderWeirdGradient(GlobalBackbuffer, offsetX, offsetY);
                RenderRectangle(GlobalBackbuffer, 0, 100, 400, 400);
                window_dimension dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, 
                                           dimension.width, dimension.height, 
                                           GlobalBackbuffer);
                offsetX += 1;
                offsetY += 2;
            }
        }
        else
        {
            OutputDebugStringA("FAILED to create a Window\n");
        }
        
    }
    else
    {
        OutputDebugStringA("FAILED to register WindowClass\n");
    }
    
    
    return(0);
}