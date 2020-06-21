#include <windows.h>
#include <stdint.h>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>
#include "typedefines.c"
#include "win32_main.h"

Global bool GlobalRunning = true;
Global user_input GlobalUserInput;
Global win32_offscreen_buffer GlobalBackbuffer;
Global LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

#include "software_render.cpp"
#include "win32_fileio.cpp"

#define Pi32 3.14159265359f

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

int 
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, i32 ShowCode)
{
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

    // NOTE: Read obj file
    file objFile = ReadEntireFile("obj.obj");
    OutputDebugStringA((LPCSTR)objFile.contents);

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

        offset.y += GlobalUserInput.up;
        offset.y -= GlobalUserInput.down;
        offset.x += GlobalUserInput.right;
        offset.x -= GlobalUserInput.left;
        v4 lineColor = {0,255,255,255};

        DrawRectangle(0, 0, 1280, 720, 0);

        // DrawGradient(offsetX, offsetY);
        // DrawLineFirst({100, 200}, {500, 600}, lineColor);
        // DrawLineSecond({200, 200}, {600, 600}, lineColor);
        DrawLineFinal(10, 100, 500 + offset.x, 500 + offset.y, {0,200,100});

        window_dimension dimension = Win32GetWindowDimension(Window);
        Win32DrawBufferToScreen(DeviceContext, 
                                   dimension.width, dimension.height, 
                                   &GlobalBackbuffer);
    }
    
    
    
    return(0);
}