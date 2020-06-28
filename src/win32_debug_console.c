//
// NOTE: Only for debugging, danger of overflowing the buffer
//

// Example Printout: 
// INFO: test1 | ..\src\win32_main.cpp: WinMain 156
// ERROR: test2 | ..\src\win32_main.cpp: WinMain 157
// SUCCESS: test3 | ..\src\win32_main.cpp: WinMain 158


#define logInfo(text, ...) PrivateLogExtra("INFO: ", text, __VA_ARGS__, __FILE__,  __FUNCTION__, __LINE__)
#define logError(text, ...) PrivateLogExtra("ERROR: ", text, __VA_ARGS__, __FILE__,  __FUNCTION__, __LINE__)
#define logSuccess(text, ...) PrivateLogExtra("SUCCESS: ", text, __VA_ARGS__, __FILE__,  __FUNCTION__, __LINE__)


#define TEXT_BUFFER_SIZE 2048
static HANDLE GLOBALConsoleHandle;
static char GLOBALRandomAccessTextBuffer1[TEXT_BUFFER_SIZE];
static char GLOBALRandomAccessTextBuffer2[TEXT_BUFFER_SIZE];


internal void 
log(char *text, ...)
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

    sprintf(GLOBALRandomAccessTextBuffer1, prepend);
    sprintf(GLOBALRandomAccessTextBuffer2, text);
    sprintf(GLOBALRandomAccessTextBuffer2 + textLength, " | %%s: %%s %%d");

    // length of " | %%s: %%s %%d"
    textLength += 12;

    va_list args;
    va_start(args, text);
    vsprintf(GLOBALRandomAccessTextBuffer1 + prependLength, GLOBALRandomAccessTextBuffer2, args);
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