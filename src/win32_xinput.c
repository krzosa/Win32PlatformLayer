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
        Log("xinput1_4.dll Failed");
    }

    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
        LogError("xinput9_1_0.dll Failed");
    }

    if(XInputLibrary)
    {
        XInputGetStateFunctionPointer = (XInputGetStateProc *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetStateFunctionPointer) 
        {
            XInputGetStateFunctionPointer = XInputGetStateStub; 
            LogError("to load XInput GetState");
            return;
        }

        XInputSetStateFunctionPointer = (XInputSetStateProc *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetStateFunctionPointer) 
        {
            XInputSetStateFunctionPointer = XInputSetStateStub;
            LogError("to load XInput SetState");
            return;
        }

        LogSuccess("XInput loaded");
    }
    else
    {
        LogError("XINPUT library load");
    }
}

internal void
Win32UpdateXInput(void)
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

