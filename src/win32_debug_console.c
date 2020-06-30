//
// NOTE: Only for debugging, danger of overflowing the buffer
// example of usage: LogInfo("test test %d", 123);
//

// Example Printout: 
// Win32MainWindowCallback 40 INFO: WM_ACTIVATEAPP
// Win32LoadXInput 47 SUCCESS: XInput loaded
// WinMain 153 INFO: OPENGL VERSION: 3.3.0 NVIDIA 445.75
// WinMain 155 INFO: test1 12 aasafaf
// WinMain 156 ERROR: test2
// WinMain 157 SUCCESS: test3

internal void Log(char *text, ...);
#define LogInfo(text, ...) PrivateLogExtra("INFO: ", text, __VA_ARGS__)
#define LogSuccess(text, ...) PrivateLogExtra("SUCCESS: ", text, __VA_ARGS__)
#define LogError(text, ...) PrivateLogExtra("%s %d ERROR: ", text, __FUNCTION__, __LINE__, __VA_ARGS__)

#define TEXT_BUFFER_SIZE 2048
static HANDLE GLOBALConsoleHandle;
static char GLOBALRandomAccessTextBuffer1[TEXT_BUFFER_SIZE];
static char GLOBALRandomAccessTextBuffer2[TEXT_BUFFER_SIZE];

internal void 
Log(char *text, ...)
{
    va_list args;
    va_start(args, text);
    vsprintf(GLOBALRandomAccessTextBuffer1, text, args);
    va_end(args);

    int textLength = CharLength(GLOBALRandomAccessTextBuffer1);
    WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer1, textLength, 0, 0);
}

internal void
PrivateLogExtra(char *prepend, char *text, ...)
{
    int textLength = CharLength(text);
    int prependLength = CharLength(prepend);

    memcpy(GLOBALRandomAccessTextBuffer2, prepend, prependLength);
    memcpy(GLOBALRandomAccessTextBuffer2 + prependLength, text, textLength);
    GLOBALRandomAccessTextBuffer2[prependLength + textLength] = '\0';

    va_list args;
    va_start(args, text);
    vsprintf(GLOBALRandomAccessTextBuffer1, GLOBALRandomAccessTextBuffer2, args);
    va_end(args);

    textLength = CharLength(GLOBALRandomAccessTextBuffer1);
    GLOBALRandomAccessTextBuffer1[textLength] = '\n';

    WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer1, textLength + 1, 0, 0);
}

internal void
Win32ConsoleAttach(void)
{
    // NOTE: Console Setup
    if(!AttachConsole(ATTACH_PARENT_PROCESS)) OutputDebugStringA("Failed to attach to console\n");

    GLOBALConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(!GLOBALConsoleHandle) OutputDebugStringA("Failed to get console handle\n");
}