// returns -1 on error
internal i64
Win32FileGetSize(char *filename)
{
    Assert(CharLength(filename) < 260); // MAX PATH

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
    Assert(CharLength(filename) < 260); // MAX PATH

    // OPEN EXISTING flag doesnt create a file
    // Creates a file if it doesnt exist
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0,
                                    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        LogError("INVALID HANDLE VALUE");
        return -1;
    }

    DWORD bytesRead;
    bool32 result = ReadFile(fileHandle, memory, (DWORD)bytesToRead, &bytesRead, 0);
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

internal files
Win32DirectoryReadAllFiles(char *directory, void *memory, i64 bytesToRead)
{
    files result = {0};
    u64 totalSizeOfFiles = 0;
    WIN32_FIND_DATAA findData;

    str8 *query = StringConcatChar(directory, "/*");

    LogInfo("Load all files in directory %s", directory);
    HANDLE handle = FindFirstFileA(query, &findData);
    if(handle == INVALID_HANDLE_VALUE)
    {
        LogError("INVALID HANDLE VALUE");
    }
    else
    {
        LogSuccess("FindFirstFileA %s %lld", findData.cFileName, findData.nFileSizeLow);
    }

    while(FindNextFileA(handle, &findData) != 0)
    {
        LogSuccess("FindNextFile %s %lld", findData.cFileName, findData.nFileSizeLow);
        if(findData.nFileSizeLow == 0) continue; // skip empty files and ".."

        result.files[result.fileCount].file = (i8 *)memory;
        result.files[result.fileCount].file += totalSizeOfFiles;

        if(totalSizeOfFiles + findData.nFileSizeLow < bytesToRead)
        {
            str8 *slash = StringConcatChar(directory, "/");
            str8 *filePath = StringConcatChar(slash, findData.cFileName);

            // FUNCTION: Copy file name 
            { 
                size_t length = CharLength(findData.cFileName);
                size_t i = 0;
                for(; i < length; i++)
                {
                    result.files[result.fileCount].fileName[i] = findData.cFileName[i];
                }
                result.files[result.fileCount].fileName[i] = '\0';
            }

            result.files[result.fileCount].fileSize = 
                Win32FileRead(filePath, result.files[result.fileCount].file, findData.nFileSizeLow);
            
            totalSizeOfFiles += result.files[result.fileCount].fileSize;
            result.fileCount++;

            StringFree(slash);
            StringFree(filePath);
        }
        else
        {
            LogError("Failed to load some of the files because passed in bytesToRead is too small for all the files to fit");
            break;
        }
    }

    DWORD message = GetLastError();
    if(message != ERROR_NO_MORE_FILES) LogError("FindNextFile %lu", message);

    FindClose(handle);
    StringFree(query);

    result.memoryFilled = totalSizeOfFiles;
    return result;
}

internal str8 *
Win32GetExecutableDirectory()
{
    char fileName[1024];

    // if this parameter is NULL, GetModuleFileName retrieves the path of the executable file of the current process.
    DWORD result = GetModuleFileNameA(0, fileName, 1024);
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