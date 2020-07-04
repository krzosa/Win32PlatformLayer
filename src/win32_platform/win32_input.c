// NOTE: Functions as types
typedef DWORD WINAPI XInputGetStateProc(DWORD dw_user_index, XINPUT_STATE *p_state);
typedef DWORD WINAPI XInputSetStateProc(DWORD dw_user_index, XINPUT_VIBRATION *p_vibration);

// NOTE: Empty (stub) functions as replacements
DWORD WINAPI XInputGetStateStub(DWORD dw_user_index, XINPUT_STATE *p_state){return ERROR_DEVICE_NOT_CONNECTED;}
DWORD WINAPI XInputSetStateStub(DWORD dw_user_index, XINPUT_VIBRATION *p_vibration){return ERROR_DEVICE_NOT_CONNECTED;}

// NOTE: Pointers to loaded functions
static XInputSetStateProc *XInputSetStateFunctionPointer = XInputSetStateStub;
static XInputGetStateProc *XInputGetStateFunctionPointer = XInputGetStateStub;

typedef enum keyboard_keys
{
    KEY_W,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F12,

    KEY_COUNT,
} keyboard_keys;

typedef enum controller_buttons
{
    BUTTON_A,
    BUTTON_B,
    BUTTON_START,
    BUTTON_RESET,

    BUTTON_COUNT,
} controller_buttons;

typedef struct user_input_controller
{
    f32 stickX;
    f32 stickY;
    
    bool8 currentButtonState[BUTTON_COUNT];
    bool8 previousButtonState[BUTTON_COUNT];
} user_input_controller;

typedef struct user_input_keyboard
{
    bool8 currentKeyState[KEY_COUNT];
    bool8 previousKeyState[KEY_COUNT];
} user_input_keyboard;

typedef struct user_input
{
    #define MAX_CONTROLLER_COUNT 4
    user_input_controller controller[MAX_CONTROLLER_COUNT];

    user_input_keyboard keyboard;
} user_input;


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

internal void
Win32XInputUpdate(void)
{
    DWORD xinputState;    

    // NOTE: i = controller index
    for (DWORD i=0; i < XUSER_MAX_COUNT; i++ )
    {
        XINPUT_STATE state;
        if(XInputGetStateFunctionPointer( i, &state ) == ERROR_SUCCESS)
        {
            XINPUT_GAMEPAD *gamepad = &state.Gamepad;
            bool8 up = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool8 down = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool8 left = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool8 right = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool8 start = (gamepad->wButtons & XINPUT_GAMEPAD_START);
            bool8 back = (gamepad->wButtons & XINPUT_GAMEPAD_BACK);
            bool8 leftShoulder = (gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool8 rightShoulder = (gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool8 AButton = (gamepad->wButtons & XINPUT_GAMEPAD_A);
            bool8 BButton = (gamepad->wButtons & XINPUT_GAMEPAD_B);
            bool8 XButton = (gamepad->wButtons & XINPUT_GAMEPAD_X);
            bool8 YButton = (gamepad->wButtons & XINPUT_GAMEPAD_Y);

            i16 stickX = gamepad->sThumbLX;
            i16 stickY = gamepad->sThumbLY;
            // LogInfo("Controller %d conntected\n", i);
        }
        else
        {
            // LogInfo("Controller %d not connected ", i);
        }
    }
}

internal bool32
IsKeyPressedOnce(user_input_keyboard *keyboard, keyboard_keys KEY)
{
    if(keyboard->previousKeyState[KEY] == 0 &&
        keyboard->currentKeyState[KEY] == 1)
    {
        keyboard->previousKeyState[KEY] = 1;
        return true;
    }
    return false;
}

internal bool32
IsKeyUnpressedOnce(user_input_keyboard *keyboard, keyboard_keys KEY)
{
    if(keyboard->previousKeyState[KEY] == 1 &&
        keyboard->currentKeyState[KEY] == 0)
    {
        keyboard->previousKeyState[KEY] = 0;
        return true;
    }
    return false;
}

internal bool32
IsKeyDown(user_input_keyboard *keyboard, keyboard_keys KEY)
{
    if(keyboard->currentKeyState[KEY] == 1)
    {
        return true;
    }
    return false;
}

internal bool32
IsKeyUp(user_input_keyboard *keyboard, keyboard_keys KEY)
{
    if(keyboard->currentKeyState[KEY] == 0)
    {
        return true;
    }
    return false;
}


internal void
Win32InputUpdate(user_input_keyboard *keyboard)
{
    MSG message;
    while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))   
    {
        u32 VKCode = message.wParam;
        switch (message.message)
        {
            case WM_KEYUP:
            case WM_KEYDOWN:
            {
                bool8 isKeyDown = (message.message == WM_KEYDOWN);
                bool8 wasKeyDown = !!(message.lParam & (1 << 30));
                if(VKCode == 'W')
                {
                    keyboard->previousKeyState[KEY_W] = wasKeyDown;
                    keyboard->currentKeyState[KEY_W] = isKeyDown;
                }
                if(VKCode == 'S')
                {
                    keyboard->previousKeyState[KEY_S] = wasKeyDown;
                    keyboard->currentKeyState[KEY_S] = isKeyDown;
                }
                if(VKCode == 'A')
                {
                    keyboard->previousKeyState[KEY_A] = wasKeyDown;
                    keyboard->currentKeyState[KEY_S] = isKeyDown;
                }
                if(VKCode == 'D')
                {
                    keyboard->previousKeyState[KEY_D] = wasKeyDown;
                    keyboard->currentKeyState[KEY_D] = isKeyDown;
                }
                if(VKCode == VK_ESCAPE)
                {
                    GLOBALAppStatus = isKeyDown;
                }
                else
                {
                    // NOTE: other keys
                }
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

