// NOTE: prototypes for function pointers
typedef void Initialize(AppMemory *memory); // called at the beginning of the app
typedef void HotReload(AppMemory *memory); // called on hot reload
typedef void Update(AppMemory *memory); // called on every frame

// NOTE: empty functions meant to be replacements when
// functions from the dll fail to load
void InitializeStub(AppMemory *memory){}
void HotReloadStub(AppMemory *memory){}
void UpdateStub(AppMemory *memory){}

typedef struct Win32DLLCode
{
    HMODULE library;    
    FILETIME lastDllWriteTime;
    bool32 isValid;

    // NOTE: pointers to functions from the dll
    Initialize *initialize;
    HotReload *hotReload;
    Update *update;
} Win32DLLCode;

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
internal Win32DLLCode
Win32DLLCodeLoad(char *mainDllPath, char *tempDllPath)
{
    Win32DLLCode result;
    result.lastDllWriteTime = Win32LastWriteTimeGet(tempDllPath);

    CopyFileA((LPCSTR)mainDllPath, (LPCSTR)tempDllPath, FALSE);
    
    result.library = LoadLibraryA(tempDllPath);
    result.isValid = 1;

    // NOTE: Load the functions from the game dll
    if (result.library)
    {
        result.initialize = (Initialize *)GetProcAddress(result.library, "Initialize");
        result.hotReload = (HotReload *)GetProcAddress(result.library, "HotReload");
        result.update = (Update *)GetProcAddress(result.library, "Update");

        result.isValid = (result.update != 0) &&
                         (result.hotReload != 0) && 
                         (result.initialize != 0);
    }

    // NOTE: if functions failed to load, load the stubs
    if (result.isValid == 0)
    {
        result.update = UpdateStub;
        result.initialize = InitializeStub;
        result.hotReload = HotReloadStub;
        
        LogError("FAILED TO LOAD LIBRARY");
    }

    LogInfo("DLLCode valid? = %d", result.isValid);
    return result;
}

/* Unloads the dll and nulls the pointers to functions from the dll */
internal void
Win32DLLCodeUnload(Win32DLLCode *dllCode)
{
    if (dllCode->library)
    {
        FreeLibrary(dllCode->library);
        dllCode->library = 0;
        dllCode->initialize = InitializeStub;
        dllCode->hotReload = HotReloadStub;
        dllCode->update = UpdateStub;
        LogInfo("Unload game code");
    }

    dllCode->isValid = false;
}