// NOTE: prototypes for function pointers
typedef void Initialize(operating_system_interface *memory); // called at the beginning of the app
typedef void HotReload(operating_system_interface *memory); // called on hot reload
typedef void HotUnload(operating_system_interface *memory); // called on hot reload
typedef void Update(operating_system_interface *memory); // called on every frame

// NOTE: empty functions meant to be replacements when
// functions from the dll fail to load
void InitializeStub(operating_system_interface *memory){}
void HotReloadStub(operating_system_interface *memory){}
void HotUnloadStub(operating_system_interface *memory){}
void UpdateStub(operating_system_interface *memory){}

typedef struct win32_dll_code
{
    HMODULE library;    
    FILETIME lastDllWriteTime;
    bool32 isValid;

    // NOTE: pointers to functions from the dll
    Initialize *initialize;
    HotReload *hotReload;
    HotUnload *hotUnload;
    Update *update;
} win32_dll_code;

/* Searches for a file, extracts properties and returns the time
    the file was last written to  */
internal FILETIME 
Win32LastWriteTimeGet(char* file)
{
    FILETIME lastWriteTime = {0};
    WIN32_FIND_DATAA data;
    HANDLE dllFileHandle = FindFirstFileA(file, &data);
    if(dllFileHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(dllFileHandle);
        lastWriteTime = data.ftLastWriteTime;
    }
    else
    {
        LogError("Invalid handle value!");
    }

    return lastWriteTime;
}

// Creates a copy of the main dll, and loads that copy
// if load fails it substitutes the loaded function with a stub(empty function)
internal win32_dll_code
Win32DLLCodeLoad(char *mainDllPath, char *tempDllPath)
{
    win32_dll_code result;
    result.lastDllWriteTime = Win32LastWriteTimeGet(tempDllPath);

    CopyFileA((LPCSTR)mainDllPath, (LPCSTR)tempDllPath, FALSE);
    
    result.library = LoadLibraryA(tempDllPath);
    result.isValid = 1;

    // NOTE: Load the functions from the game dll
    if (result.library)
    {
        result.initialize = (Initialize *)GetProcAddress(result.library, "Initialize");
        result.hotReload = (HotReload *)GetProcAddress(result.library, "HotReload");
        result.hotUnload = (HotUnload *)GetProcAddress(result.library, "HotUnload");
        result.update = (Update *)GetProcAddress(result.library, "Update");

        result.isValid = (result.update != 0) &&
                         (result.hotReload != 0) && 
                         (result.hotUnload != 0) && 
                         (result.initialize != 0);
    }

    // NOTE: if functions failed to load, load the stubs
    if (result.isValid == 0)
    {
        result.update = UpdateStub;
        result.initialize = InitializeStub;
        result.hotReload = HotReloadStub;
        result.hotUnload = HotUnloadStub;
        
        LogError("MainDLLCode Load");
    }

    LogInfo("MainDLLCode Load");
    return result;
}

/* Unloads the dll and nulls the pointers to functions from the dll */
internal void
Win32DLLCodeUnload(win32_dll_code *dllCode)
{
    if (dllCode->library)
    {
        FreeLibrary(dllCode->library);
        dllCode->library = 0;
        dllCode->initialize = InitializeStub;
        dllCode->hotReload = HotReloadStub;
        dllCode->hotUnload = HotReloadStub;
        dllCode->update = UpdateStub;
        LogInfo("MainDLLCode Unload");
    }

    dllCode->isValid = false;
}

internal void
Win32UpdateDLLCode(win32_dll_code *dllCode, char *mainDLLPath, char *tempDLLPath, operating_system_interface *os)
{
    // NOTE: Check if dll was rebuild and load it again if it did
    FILETIME newDLLWriteTime = Win32LastWriteTimeGet(mainDLLPath);
    if(CompareFileTime(&newDLLWriteTime, &dllCode->lastDllWriteTime) != 0)
    {
        dllCode->hotUnload(os);

        Win32DLLCodeUnload(dllCode);
        Sleep(100);

        *dllCode = Win32DLLCodeLoad(mainDLLPath, tempDLLPath);

        // NOTE: Call HotReload function from the dll
        dllCode->hotReload(os);
    }
}