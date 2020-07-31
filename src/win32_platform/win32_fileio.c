// returns 0 on error
internal u64
Win32FileGetSize(char *filename)
{
    Assert(CharLength(filename) < MAX_PATH); // MAX PATH

    LARGE_INTEGER size;

    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        size.QuadPart = 0;
        LogError("INVALID HANDLE VALUE %s", filename);
        return size.QuadPart;
    }
    bool32 result = GetFileSizeEx(fileHandle, &size);
    if(!result)
    {
        size.QuadPart = 0;
        LogError("GetFileSize failed %s", filename);
        return size.QuadPart;
    }

    CloseHandle(fileHandle);
    return size.QuadPart;
}

// Fills the specified memory buffer with the file contents
// returns 0 on fail, else returns bytesRead
internal u64
Win32FileRead(char *filename, void *memory, u64 bytesToRead)
{
    Assert(CharLength(filename) < MAX_PATH); // MAX PATH

    // OPEN EXISTING flag doesnt create a file
    // Creates a file if it doesnt exist
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0,
                                    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        LogError("INVALID HANDLE VALUE");
        return 0;
    }

    DWORD bytesRead;
    bool32 result = ReadFile(fileHandle, memory, (DWORD)bytesToRead, &bytesRead, 0);
    if(!result)
    {
        LogError("ReadFile failed %s", filename);
        return 0;
    }
    if(bytesRead != bytesToRead)
    {
        LogError("BytesRead and BytesToRead mismatch %s", filename);
        return 0;
    }

    LogSuccess("%s FILE LOADED", filename);
    return bytesRead;
}

internal files
Win32DirectoryReadAllFiles(char *directory, void *memory, u64 bytesToRead)
{
    files result = {0};
    u64 totalSizeOfFiles = 0;
    WIN32_FIND_DATAA findData;

    str8 *query = StringConcatChar(directory, "/*");
    Assert(StringLength(query) < MAX_PATH);

    LogInfo("Load all files in directory %s", directory);
    HANDLE handle = FindFirstFileA(query, &findData);
    if(handle == INVALID_HANDLE_VALUE)
    {
        LogError("INVALID HANDLE VALUE %s", directory);
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
                memcpy(result.files[result.fileCount].fileName, findData.cFileName, length);
                result.files[result.fileCount].fileName[length] = '\0';
                result.files[result.fileCount].fileNameLength = (u32)length;
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