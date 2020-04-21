#include <windows.h>

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR     lpCmdLine,
            int       nShowCmd)
{
    MessageBoxA(NULL, "hello world", "title",
        MB_OK|MB_ICONINFORMATION);   
    return 0;
}