// NOTE: Console attach
#define TEXT_BUFFER_SIZE 2048
static HANDLE GLOBALConsoleHandle;
static char GLOBALRandomAccessTextBuffer[TEXT_BUFFER_SIZE];

// NOTE: Only for debugging, danger of overflowing the buffer

// Print stuff to console
#define log(text, ...) { \
    sprintf_s(GLOBALRandomAccessTextBuffer, TEXT_BUFFER_SIZE, text, __VA_ARGS__); \
    WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer, CharLength(GLOBALRandomAccessTextBuffer), 0, 0); }

// Print stuff to console, prepend Log: && append a new line
#define logInfo(text, ...) { \
    sprintf_s(GLOBALRandomAccessTextBuffer, 7, "INFO: ");\
    sprintf_s((GLOBALRandomAccessTextBuffer + 6), TEXT_BUFFER_SIZE, text, __VA_ARGS__); \
    int length = CharLength(GLOBALRandomAccessTextBuffer); \
    GLOBALRandomAccessTextBuffer[length] = '\n'; \
    GLOBALRandomAccessTextBuffer[length + 1] = '\0'; \
    WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer, length + 1, 0, 0); }

// Print stuff to console, prepend Error: && append a new line
#define logError(text, ...) { \
    sprintf_s(GLOBALRandomAccessTextBuffer, 8, "ERROR: ");\
    sprintf_s((GLOBALRandomAccessTextBuffer + 7), TEXT_BUFFER_SIZE, text, __VA_ARGS__); \
    int length = CharLength(GLOBALRandomAccessTextBuffer); \
    GLOBALRandomAccessTextBuffer[length] = '\n'; \
    GLOBALRandomAccessTextBuffer[length + 1] = '\0'; \
    WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer, length + 1, 0, 0); } 

// Print stuff to console, prepend Success: && append a new line
#define logSuccess(text, ...) { \
    sprintf_s(GLOBALRandomAccessTextBuffer, 10, "SUCCESS: ");\
    sprintf_s((GLOBALRandomAccessTextBuffer + 9), TEXT_BUFFER_SIZE, text, __VA_ARGS__); \
    int length = CharLength(GLOBALRandomAccessTextBuffer); \
    GLOBALRandomAccessTextBuffer[length] = '\n'; \
    GLOBALRandomAccessTextBuffer[length + 1] = '\0'; \
    WriteConsole(GLOBALConsoleHandle, GLOBALRandomAccessTextBuffer, length + 1, 0, 0); }

internal void
Win32ConsoleAttach(void)
{
    // NOTE: Console Setup
    if(!AttachConsole(ATTACH_PARENT_PROCESS)) OutputDebugStringA("Failed to attach to console\n");

    GLOBALConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(!GLOBALConsoleHandle) OutputDebugStringA("Failed to get console handle\n");
}