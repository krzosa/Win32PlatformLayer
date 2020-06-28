#define TEXT_BUFFER_SIZE 2048
static HANDLE GLOBALConsoleHandle;
static char GLOBALRandomAccessTextBuffer[TEXT_BUFFER_SIZE];

// NOTE: Only for debugging, danger of overflowing the buffer

#define logInfo(text, ...) logPrepend("INFO: ", text, __VA_ARGS__)
#define logError(text, ...) logPrepend("ERROR: ", text, __VA_ARGS__)
#define logSuccess(text, ...) logPrepend("SUCCESS: ", text, __VA_ARGS__)

internal void 
log(char *text, ...)
{
    va_list args;
    va_start(args, text);
    vsprintf(GLOBALRandomAccessTextBuffer, text, args);
    va_end(args);

    int textLength = CharLength(GLOBALRandomAccessTextBuffer);
    WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer, textLength, 0, 0);
}

internal void
logPrepend(char *prepend, char *text, ...)
{
    size_t prependSize = CharLength(prepend);
    sprintf(GLOBALRandomAccessTextBuffer, prepend);

    va_list args;
    va_start(args, text);
    vsprintf(GLOBALRandomAccessTextBuffer + prependSize, text, args);
    va_end(args);

    int textLength = CharLength(GLOBALRandomAccessTextBuffer);
    GLOBALRandomAccessTextBuffer[textLength] = '\n';

    WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer, textLength + 1, 0, 0);
}

internal void
Win32ConsoleAttach(void)
{
    // NOTE: Console Setup
    if(!AttachConsole(ATTACH_PARENT_PROCESS)) OutputDebugStringA("Failed to attach to console\n");

    GLOBALConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(!GLOBALConsoleHandle) OutputDebugStringA("Failed to get console handle\n");
}