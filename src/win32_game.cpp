#include <windows.h>

#define global_variable static
// localy scoped variable that will persist its value
// when function goes out of scope
#define local_persist static
// reserved for functions that are going to
// be internal to this file, local functions
#define internal static

// loop variable
global_variable bool Running = true;

// The BITMAPINFO structure defines the 
// dimensions and color information for a DIB.
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;


internal void
Win32ResizeDIBSection(int Width, int Height)
{
    if(BitmapHandle)
    {
        DeleteObject(BitmapHandle);
    }

    if(!BitmapDeviceContext)
    {
        // TODO(casey): Should we recreate these under certain special circumstances
        BitmapDeviceContext = CreateCompatibleDC(0);
    }
    
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    // The CreateDIBSection function creates a DIB 
    // that applications can write to directly. The function gives 
    // you a pointer to the location of the bitmap bit values. 
    BitmapHandle = CreateDIBSection(
        BitmapDeviceContext, &BitmapInfo,
        DIB_RGB_COLORS,
        &BitmapMemory,
        0, 0);
}

internal void
Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
    // SRCCOPY - copy directly
    StretchDIBits(
        DeviceContext,
        X, Y, Width, Height,
        X, Y, Width, Height,
        BitmapMemory,
        &BitmapInfo,
        DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(
   HWND   Window,
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
            RECT ClientRect;
            // get size of the window, without the border
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
        } break;
        // WM_CLOSE is called when user sends signal to terminate the application
        // we can handle the closing procedure here
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            Running = false;
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
            Running = false;
        } break;

        case WM_PAINT:
        {
            // The PAINTSTRUCT structure contains information for an application. 
            // This information can be used to paint the client area of a window owned by that application.
            PAINTSTRUCT Paint;
            HDC DeviceContext;
            // The BeginPaint function prepares the specified window for 
            // painting and fills a PAINTSTRUCT structure with information about the painting.
            DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            
            Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        } break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
        
    }

    return Result   ;
}


int CALLBACK
WinMain(HINSTANCE Instance,
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
        HWND WindowHandle = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "TITLE_PLACEHOLDER",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, Instance, NULL);
        if(WindowHandle)
        {
            while(Running)
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
                    Running = false;
                }
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
