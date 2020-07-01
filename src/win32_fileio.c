internal void
FreeFileMemory(void *Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

internal file_data 
ReadEntireFile(char *filename)
{
    file_data result = {0};
    
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(fileHandle, &FileSize))
        {
            u32 fileSize32 = (u32)FileSize.QuadPart;
            result.contents = VirtualAlloc(0, fileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(result.contents)
            {
                DWORD BytesRead;
                if(ReadFile(fileHandle, result.contents, fileSize32, &BytesRead, 0) && (fileSize32 == BytesRead))
                {
                    result.size = fileSize32;
                    Log("ReadEntireFile: Success\n");
                }
                else
                {                    
                    Log("ReadEntireFile: ReadFile failed\n");
                    FreeFileMemory(result.contents);
                    result.contents = 0;
                }
            }
            else
            {
                Log("ReadEntireFile: NULL file contents\n");
            }
        }
        else
        {
            Log("ReadEntireFile: Failed to get file size\n");
        }

        CloseHandle(fileHandle);
    }
    else
    {
        Log("ReadEntireFile: Invalid file handle name\n");
    }

    return(result);
}

internal str *
Win32ExecutableDirectoryPathGet()
{
    char fileName[MAX_PATH];
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