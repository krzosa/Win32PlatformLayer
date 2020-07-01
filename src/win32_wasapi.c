// NOTE: typedefines for the functions which are goint to be loaded
typedef HRESULT CoCreateInstanceFunction (REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
typedef HRESULT CoInitializeExFunction (LPVOID pvReserved, DWORD dwCoInit);

// NOTE: empty functions(stubs) which are used when library fails to load
HRESULT CoCreateInstanceStub(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv){return 0;}
HRESULT CoInitializeExStub(LPVOID pvReserved, DWORD dwCoInit){return 0;}

// NOTE: pointers to the functions from the dll
static CoCreateInstanceFunction *CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
static CoInitializeExFunction *CoInitializeExFunctionPointer = CoInitializeExStub;

internal void
Win32COMLoad(void)
{
    HMODULE ole32Library = LoadLibraryA("ole32.dll");
    if(ole32Library)
    {
        LogSuccess("COM Ole32.dll load");
        CoCreateInstanceFunctionPointer = (CoCreateInstanceFunction *)GetProcAddress(ole32Library, "CoCreateInstance");
        if(!CoCreateInstanceFunctionPointer)
        {
            LogError("CoCreateInstance load");
            CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
        }
        CoInitializeExFunctionPointer = (CoInitializeExFunction *)GetProcAddress(ole32Library, "CoInitializeEx");
        if(!CoInitializeExFunctionPointer)
        {
            LogError("CoInitializeEx load");
            CoInitializeExFunctionPointer = CoInitializeExStub;
        }
    }
    else
    {
        LogError("COM Ole32.dll load");
        CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
        CoInitializeExFunctionPointer = CoInitializeExStub;
    }
}

internal void
Win32WasapiInitialize()
{
    CoInitializeExFunctionPointer(0, COINIT_SPEED_OVER_MEMORY);

    HRESULT result;

    IMMDeviceEnumerator *deviceEnum = NULL;
    result = CoCreateInstanceFunctionPointer(
        &CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, &IID_IMMDeviceEnumerator,
        (LPVOID *)&deviceEnum);

    if(result != S_OK)
    {
        LogError("CoCreateInstance");
        return;
    }

    IMMDevice *device = NULL;
    result = deviceEnum->lpVtbl->GetDefaultAudioEndpoint(deviceEnum, eRender, eConsole, &device);

    if(result != S_OK)
    {
        LogError("GetDefaultAudioEndpoint");
        return;
    }

    IAudioClient *audioClient;
    result = device->lpVtbl->Activate(device, &IID_IAudioClient, CLSCTX_ALL, 0, (void **)&audioClient);

    if(result != S_OK)
    {
        LogError("IAudioClient Activate");
        return;
    }

    WAVEFORMATEX *waveFormat = 0;
    audioClient->lpVtbl->GetMixFormat(audioClient, &waveFormat);

}