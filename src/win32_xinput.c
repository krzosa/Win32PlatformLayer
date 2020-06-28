// Functions as types
typedef DWORD WINAPI XInputGetStateProc(DWORD dw_user_index, XINPUT_STATE *p_state);
typedef DWORD WINAPI XInputSetStateProc(DWORD dw_user_index, XINPUT_VIBRATION *p_vibration);

// Empty (stub) functions as replacements
DWORD WINAPI XInputGetStateStub(DWORD dw_user_index, XINPUT_STATE *p_state){return ERROR_DEVICE_NOT_CONNECTED;}
DWORD WINAPI XInputSetStateStub(DWORD dw_user_index, XINPUT_VIBRATION *p_vibration){return ERROR_DEVICE_NOT_CONNECTED;}

static XInputSetStateProc *XInputSetStateFunctionPointer = XInputSetStateStub;
static XInputGetStateProc *XInputGetStateFunctionPointer = XInputGetStateStub;

internal void 
Win32LoadXInput(void)    
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");

    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
        log("xinput1_4.dll Failed");
    }

    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
        log("xinput9_1_0.dll Failed");
    }

    if(XInputLibrary)
    {
        XInputGetStateFunctionPointer = (XInputGetStateProc *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetStateFunctionPointer) 
        {
            XInputGetStateFunctionPointer = XInputGetStateStub; 
            log("FAILED: to load XInput GetState\n");
            return;
        }

        XInputSetStateFunctionPointer = (XInputSetStateProc *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetStateFunctionPointer) 
        {
            XInputSetStateFunctionPointer = XInputSetStateStub;
            log("FAILED: to load XInput SetState\n");
            return;
        }

        log("SUCCESS: XInput loaded\n");
    }
    else
    {
        log("FAILED: XINPUT library load\n")
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
            bool up = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool down = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool left = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool right = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool start = (gamepad->wButtons & XINPUT_GAMEPAD_START);
            bool back = (gamepad->wButtons & XINPUT_GAMEPAD_BACK);
            bool leftShoulder = (gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool rightShoulder = (gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool AButton = (gamepad->wButtons & XINPUT_GAMEPAD_A);
            bool BButton = (gamepad->wButtons & XINPUT_GAMEPAD_B);
            bool XButton = (gamepad->wButtons & XINPUT_GAMEPAD_X);
            bool YButton = (gamepad->wButtons & XINPUT_GAMEPAD_Y);

            i16 stickX = gamepad->sThumbLX;
            i16 stickY = gamepad->sThumbLY;
            // logInfo("Controller %d conntected\n", i);
        }
        else
        {
            // logInfo("Controller %d not connected ", i);
        }
    }
}

