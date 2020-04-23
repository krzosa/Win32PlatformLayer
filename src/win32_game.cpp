#include <windows.h>
#include <stdint.h>
#include "win32_game.h"

// loop variable
global_variable bool RUNNING = true;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable int bytesPerPixel = 4;

internal void
RenderWeirdGradient(int BlueOffset, int GreenOffset)
{    
    int Width = GlobalBackbuffer.width;
    int Height = GlobalBackbuffer.height;

    int Pitch = Width * bytesPerPixel;
    uint8_t *Row = (uint8_t *)GlobalBackbuffer.memory;    
    for(int Y = 0; Y < GlobalBackbuffer.height; ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for(int X = 0; X < GlobalBackbuffer.width; ++X)
        {
            uint8_t Blue = (X + BlueOffset);
            uint8_t Green = (Y + GreenOffset);
            
            *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Pitch;
    }
}

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    RECT ClientRect;
    win32_window_dimension windowDimension;
    // get size of the window, without the border
    GetClientRect(Window, &ClientRect);
    windowDimension.width = ClientRect.right - ClientRect.left;
    windowDimension.height = ClientRect.bottom - ClientRect.top;
    return windowDimension;
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer* buffer, 
                                    win32_window_dimension dimension)
{
    // if we dont free before allocating, memory will leak
    if(buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = dimension.width;
    buffer->height = dimension.height;

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
    buffer->pitch = dimension.width * bytesPerPixel;
}

internal void Win32DisplayBufferInWindow(HDC DeviceContext, 
                                        win32_window_dimension window, 
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
        0, 0, window.width, window.height,
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
        case WM_SIZE:
        {
            win32_window_dimension dimension = Win32GetWindowDimension(Window);
            Win32ResizeDIBSection(&GlobalBackbuffer, dimension);
        } break;
        // WM_CLOSE is called when user sends signal to terminate the application
        // we can handle the closing procedure here
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            RUNNING = false;
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
            RUNNING = false;
        } break;

        case WM_PAINT:
        {
            // The PAINTSTRUCT structure contains information for an application. 
            // This information can be used to paint the client area of a window owned by that application.
            PAINTSTRUCT Paint;
            // The BeginPaint function prepares the specified window for 
            // painting and fills a PAINTSTRUCT structure with information about the painting.
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dimension dimension = Win32GetWindowDimension(Window);
            
            Win32DisplayBufferInWindow(DeviceContext, dimension, GlobalBackbuffer);
            EndPaint(Window, &Paint);
        } break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
        
    }

    return Result   ;
}


int CALLBACK    WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR     CommandLine,
        int       ShowCode)
{
    WNDCLASSA WindowClass;
    WindowClass.style          = CS_HREDRAW | CS_VREDRAW;
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
        // szWindowClass: the name of the application
        // szTitle: the text that appears in the title bar
        // WS_OVERLAPPEDWINDOW: the type of window to create
        // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
        // 500, 100: initial size (width, length)
        // NULL: the parent of this window
        // NULL: this application does not have a menu bar
        // hInstance: the first parameter from WinMain
        // NULL: not used in this application
        HWND Window = CreateWindowExA(
            0,
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

            while(RUNNING)
            {
                MSG Message;
                BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
                if(MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                else
                {
                    RUNNING = false;
                }
                RenderWeirdGradient(offsetX, offsetY);

                win32_window_dimension window_dimension = 
                    Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, window_dimension, GlobalBackbuffer);
                offsetX += 1;
                offsetY += 2;
            }
        }
        else
        {
            // TODO(Karol): Logging
        }
        
    }
    else
    {
        //TODO(Karol): Logging
    }
    
    
    return(0);
}