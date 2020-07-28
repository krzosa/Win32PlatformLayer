global_variable operating_system_interface *os = 0;

internal bool32
IsKeyPressedOnce(keyboard_keys KEY)
{
    if(os->userInput.keyboard.previousKeyState[KEY] == 0 &&
        os->userInput.keyboard.currentKeyState[KEY] == 1)
    {
        os->userInput.keyboard.previousKeyState[KEY] = 1;
        return true;
    }
    return false;
}

internal bool32
IsKeyUnpressedOnce(keyboard_keys KEY)
{
    if(os->userInput.keyboard.previousKeyState[KEY] == 1 &&
        os->userInput.keyboard.currentKeyState[KEY] == 0)
    {
        os->userInput.keyboard.previousKeyState[KEY] = 0;
        return true;
    }
    return false;
}

internal bool32
IsKeyDown(keyboard_keys KEY)
{
    if(os->userInput.keyboard.currentKeyState[KEY] == 1)
    {
        return true;
    }
    return false;
}

internal bool32
IsKeyUp(keyboard_keys KEY)
{
    if(os->userInput.keyboard.currentKeyState[KEY] == 0)
    {
        return true;
    }
    return false;
}

internal bool32
IsButtonPressedOnce(controller_buttons BUTTON)
{
    if(os->userInput.controller[0].previousButtonState[BUTTON] == 0 &&
        os->userInput.controller[0].currentButtonState[BUTTON] == 1)
    {
        os->userInput.controller[0].previousButtonState[BUTTON] = 1;
        return true;
    }
    return false;
}

internal bool32
IsButtonUnpressedOnce(controller_buttons BUTTON)
{
    if(os->userInput.controller[0].previousButtonState[BUTTON] == 1 &&
        os->userInput.controller[0].currentButtonState[BUTTON] == 0)
    {
        os->userInput.controller[0].previousButtonState[BUTTON] = 0;
        return true;
    }
    return false;
}

internal bool32
IsButtonDown(controller_buttons BUTTON)
{
    if(os->userInput.controller[0].currentButtonState[BUTTON] == 1)
    {
        return true;
    }
    return false;
}

internal bool32
IsButtonUp(controller_buttons BUTTON)
{
    if(os->userInput.controller[0].currentButtonState[BUTTON] == 0)
    {
        return true;
    }
    return false;
}

internal f32
GetAppStartTimeMilliseconds()
{
    return os->timeData.startAppMilliseconds;
}

internal i64 
GetAppStartTimeCounts()
{
    return os->timeData.startAppCount;
}

internal i64 
GetAppStartTimeCycles()
{
    return os->timeData.startAppCycles;
}

//
// NOTE: Update == how long it took to process all the things in a frame without the 
//                 end frame sleep

internal f32
GetUpdateTimeMilliseconds()
{
    return os->timeData.updateMilliseconds;
}

internal i64 
GetUpdateTimeCounts()
{
    return os->timeData.updateCount;
}

internal i64 
GetUpdateTimeCycles()
{
    return os->timeData.updateCycles;
}

// NOTE: Frame == entire length of a single frame

internal f32
GetFrameTimeMilliseconds()
{
    return os->timeData.updateMilliseconds;
}

internal i64 
GetFrameTimeCounts()
{
    return os->timeData.updateCount;
}

internal i64 
GetFrameTimeCycles()
{
    return os->timeData.updateCycles;
}

inline internal f32
MillisecondsToFramesPerSecond(f32 millisecondsPerFrame)
{
    return (1 / millisecondsPerFrame) * 1000;
}

#define ConsoleLog(text, ...) os->Log(text, __VA_ARGS__)
#define ConsoleLogExtra(prepend, text, ...) os->LogExtra(prepend, text, __VA_ARGS__)