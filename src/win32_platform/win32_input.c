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