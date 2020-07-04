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