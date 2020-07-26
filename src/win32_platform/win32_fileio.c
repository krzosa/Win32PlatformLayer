// returns -1 on error
internal i64
Win32FileGetSize(char *filename)
{
    LARGE_INTEGER size;

    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        size.QuadPart = -1;
        LogError("INVALID HANDLE VALUE");
        return size.QuadPart;
    }
    bool32 result = GetFileSizeEx(fileHandle, &size);
    if(!result)
    {
        size.QuadPart = -1;
        LogError("GetFileSize failed");
        return size.QuadPart;
    }

    CloseHandle(fileHandle);
    return size.QuadPart;
}

// Fills the specified memory buffer with the file contents
// returns -1 on fail, else returns bytesRead
internal i64
Win32FileRead(char *filename, void *memory, i64 bytesToRead)
{
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        LogError("INVALID HANDLE VALUE");
        return -1;
    }

    DWORD bytesRead;
    bool32 result = ReadFile(fileHandle, memory, bytesToRead, &bytesRead, 0);
    if(!result)
    {
        LogError("ReadFile failed");
        return -1;
    }
    if(bytesRead != bytesToRead)
    {
        LogError("BytesRead and BytesToRead mismatch!");
        return -1;
    }

    LogSuccess("%s FILE LOADED", filename);
    return (i64)bytesRead;
}

internal str8 *
Win32GetExecutableDirectory()
{
    char fileName[2048];

    // if this parameter is NULL, GetModuleFileName retrieves the path of the executable file of the current process.
    DWORD result = GetModuleFileNameA(0, fileName, MAX_PATH);
    if(!result)
    {
        LogError("GetModuleFileNameA");
    }
    
    i32 i = result - 1;
    for(; i != 0; i--)
    {
        if(fileName[i] == '\\') break;
    }

    return StringCreateSubstring(fileName, i);
}