LRESULT CALLBACK 
Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;

    uint32_t VKCode = wParam;
    switch (message) 
    {
        case WM_CLOSE:
        {
            GLOBALAppStatus = false;
            break;
        } 
        case WM_DESTROY:
        {
            GLOBALAppStatus = false;
            break;
        } 
        case WM_QUIT:
        {
            GLOBALAppStatus = false;
            break;
        }

        // NOTE: resize opengl viewport on window resize
        case WM_WINDOWPOSCHANGING:
        case WM_SIZE:
        {
            Win32OpenGLAspectRatioUpdate(window, 16, 9);
            break;
        }

        case WM_KEYDOWN:
        {
            if(VKCode == 'W')
            {
                GLOBALUserInput.up = 1;
            }
            else if(VKCode == 'S')
            {
                GLOBALUserInput.down = 1;
            }
            if(VKCode == 'A')
            {
                GLOBALUserInput.left = 1;
            }
            else if(VKCode == 'D')
            {
                GLOBALUserInput.right = 1;
            }
            else if(VKCode == VK_ESCAPE)
            {
                GLOBALAppStatus = 0;
            }
            else if(VKCode == VK_F1)
            {
                GLOBALUserInput.reset = 1;
            }
            break;
        }
        case WM_KEYUP:
        {
            if(VKCode == 'W')
            {
                GLOBALUserInput.up = 0;
            }
            else if(VKCode == 'S')
            {
                GLOBALUserInput.down = 0;
            }
            if(VKCode == 'A')
            {
                GLOBALUserInput.left = 0;
            }
            else if(VKCode == 'D')
            {
                GLOBALUserInput.right = 0;
            }
            else if(VKCode == VK_F1)
            {
                GLOBALUserInput.reset = 0;
            }
            break;
        }
        default:
        {
            Result = DefWindowProc(window, message, wParam, lParam);
        } break;
        
    }

    return Result;
}