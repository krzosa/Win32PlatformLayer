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
    KEY_S,
    KEY_A,
    KEY_D,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F12,
    KEY_ESC,

    KEY_COUNT,
} keyboard_keys;

typedef enum controller_buttons
{
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,

    BUTTON_DPAD_UP,
    BUTTON_DPAD_DOWN,
    BUTTON_DPAD_LEFT,
    BUTTON_DPAD_RIGHT,

    BUTTON_LEFT_SHOULDER,
    BUTTON_RIGHT_SHOULDER,

    BUTTON_START,
    BUTTON_SELECT,

    BUTTON_COUNT,
} controller_buttons;

typedef struct user_input_controller
{
    f32 leftStickX;
    f32 leftStickY;

    f32 rightStickX;
    f32 rightStickY;
    
    bool8 connected;
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
    user_input_controller controller[XUSER_MAX_COUNT];

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

#define KEYUpdate(KEY){ \
    keyboard->previousKeyState[KEY] = wasKeyDown; \
    keyboard->currentKeyState[KEY] = isKeyDown;}

#define BUTTONUpdate(BUTTON, XINPUT_BUTTON) \
    controller->previousButtonState[BUTTON] = controller->currentButtonState[BUTTON]; \
    controller->currentButtonState[BUTTON] = (gamepad->wButtons & XINPUT_BUTTON) != 0;


internal void
Win32InputUpdate(user_input *userInput)
{
    user_input_keyboard *keyboard = &userInput->keyboard;

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

                if(VKCode == 'W') KEYUpdate(KEY_W)
                if(VKCode == 'S') KEYUpdate(KEY_S)
                if(VKCode == 'A') KEYUpdate(KEY_A)
                if(VKCode == 'D') KEYUpdate(KEY_D)
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
                else{} // NOTE: other keys
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
    DWORD xinputState;    

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

            controller->leftStickX = gamepad->sThumbLX / 32767.f;
            controller->leftStickY = gamepad->sThumbLY / 32767.f;

            controller->rightStickX = gamepad->sThumbRX / 32767.f;
            controller->rightStickY = gamepad->sThumbRY / 32767.f;

            controller->connected = 1;
            // LogInfo("Controller %d conntected", i);
        }
        else
        {
            controller->connected = 0;
            // LogInfo("Controller %d not connected ", i);
        }
    }
}