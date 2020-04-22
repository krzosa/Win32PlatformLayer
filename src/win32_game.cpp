#include <windows.h>

LRESULT CALLBACK MainWindowCallback(
   HWND   Window,
   UINT   Message,
   WPARAM WParam,
   LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message) 
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            PostQuitMessage(0);
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        case WM_QUIT:
        {
            OutputDebugStringA("WM_QUIT\n");
            PostQuitMessage(0);
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext;
            DeviceContext = BeginPaint(Window, &Paint);
            FillRect(DeviceContext, &Paint.rcPaint, (HBRUSH) (3));
            // int X = Paint.rcPaint.left;
            // int Y = Paint.rcPaint.top;
            // int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            // int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            // static DWORD Operation = BLACKNESS;            
            // PatBlt(DeviceContext, X, Y, Width, Height, Operation);
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
    WindowClass.lpfnWndProc    = MainWindowCallback;
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
            for( ;; )
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
                    break;
                }
            }
        }
    }
    
    
    return(0);
}
