static const GUID IID_IAudioClient = {0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2};
static const GUID IID_IAudioRenderClient = {0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2};
static const GUID CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E};
static const GUID IID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6};
static const GUID PcmSubformatGuid = {STATIC_KSDATAFORMAT_SUBTYPE_PCM};

// NOTE: typedefines for the functions which are goint to be loaded
typedef HRESULT CoCreateInstanceFunction(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
typedef HRESULT CoInitializeExFunction(LPVOID pvReserved, DWORD dwCoInit);

// NOTE: empty functions(stubs) which are used when library fails to load
HRESULT CoCreateInstanceStub(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv) { return 0; }
HRESULT CoInitializeExStub(LPVOID pvReserved, DWORD dwCoInit) { return 0; }

// NOTE: pointers to the functions from the dll
static CoCreateInstanceFunction *CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
static CoInitializeExFunction *CoInitializeExFunctionPointer = CoInitializeExStub;

typedef struct audio_data
{
    IMMDevice *device;
    IAudioClient *audioClient;
    IMMDeviceEnumerator *deviceEnum;
    IAudioRenderClient *audioRenderClient;

    bool32 initialized;
    u32 samplesPerSecond;
    u32 numberOfChannels;
    u32 bufferFrameCount;
    REFERENCE_TIME bufferDuration;
} audio_data;

// NOTE: Load COM Library functions dynamically, 
//       this way sound is not necessary to run the game
internal void
Win32COMLoad(void)
{
    HMODULE ole32Library = LoadLibraryA("ole32.dll");
    if (ole32Library)
    {
        LogSuccess("COM Ole32.dll load");
        CoCreateInstanceFunctionPointer = 
            (CoCreateInstanceFunction *)GetProcAddress(ole32Library, "CoCreateInstance");

        if (!CoCreateInstanceFunctionPointer)
        {
            LogError("CoCreateInstance load");
            CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
        }
        CoInitializeExFunctionPointer = 
            (CoInitializeExFunction *)GetProcAddress(ole32Library, "CoInitializeEx");

        if (!CoInitializeExFunctionPointer)
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

internal audio_data
Win32WasapiInitialize()
{
    audio_data audio = {0};

    HRESULT result;
    result = CoInitializeExFunctionPointer(0, COINIT_SPEED_OVER_MEMORY);

    if(result != S_OK)
    {
        LogError("CoInitializeExFunction");
        return audio;
    }


    result = CoCreateInstanceFunctionPointer(
        &CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, &IID_IMMDeviceEnumerator,
        (LPVOID *)&audio.deviceEnum);

    if (result != S_OK)
    {
        LogError("CoCreateInstance");
        return audio;
    }

    result = audio.deviceEnum->lpVtbl->GetDefaultAudioEndpoint(
        audio.deviceEnum, eRender, eConsole, &audio.device
    );

    if (result != S_OK)
    {
        LogError("GetDefaultAudioEndpoint");
        return audio;
    }

    result = audio.device->lpVtbl->Activate(
        audio.device, &IID_IAudioClient, CLSCTX_ALL, 0, (void **)&audio.audioClient
    );

    if (result != S_OK)
    {
        LogError("IAudioClient Activate");
        return audio;
    }

    // WAVEFORMATEX *currWaveFormat = 0;
    // audioClient->lpVtbl->GetMixFormat(audioClient, &currWaveFormat);

    audio.samplesPerSecond = 48000;
    audio.numberOfChannels = 2;
    WAVEFORMATEX waveFormat = {0};
    {
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = audio.numberOfChannels;
        waveFormat.nSamplesPerSec = audio.samplesPerSecond;
        waveFormat.wBitsPerSample = 16;
        waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize = 0;
    }

    #define REFERENCE_TIMES_PER_SEC 10000000
    REFERENCE_TIME requestedBufferDuration = REFERENCE_TIMES_PER_SEC * 2;

    result = audio.audioClient->lpVtbl->Initialize(
        audio.audioClient, AUDCLNT_SHAREMODE_SHARED, 
        AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | 
        AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, 
        requestedBufferDuration, 0, &waveFormat, 0
    );

    if(result != S_OK)
    {
        switch(result)
        {
            case AUDCLNT_E_WRONG_ENDPOINT_TYPE:{ LogError("AUDCLNT_E_WRONG_ENDPOINT_TYPE");break;}
            case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED:{ LogError("AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED");break;}
            case AUDCLNT_E_BUFFER_SIZE_ERROR:{ LogError("AUDCLNT_E_BUFFER_SIZE_ERROR");break;}
            case AUDCLNT_E_CPUUSAGE_EXCEEDED:{ LogError("AUDCLNT_E_CPUUSAGE_EXCEEDED");break;}
            case AUDCLNT_E_DEVICE_INVALIDATED:{ LogError("AUDCLNT_E_DEVICE_INVALIDATED");break;}
            case AUDCLNT_E_DEVICE_IN_USE:{ LogError("AUDCLNT_E_DEVICE_IN_USE");break;}
            case AUDCLNT_E_ENDPOINT_CREATE_FAILED:{ LogError("AUDCLNT_E_ENDPOINT_CREATE_FAILED");break;}
            case AUDCLNT_E_INVALID_DEVICE_PERIOD:{ LogError("AUDCLNT_E_INVALID_DEVICE_PERIOD");break;}
            case AUDCLNT_E_UNSUPPORTED_FORMAT:{ LogError("AUDCLNT_E_UNSUPPORTED_FORMAT");break;}
            case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:{ LogError("AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED");break;}
            case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL:{ LogError("AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL");break;}
            case AUDCLNT_E_SERVICE_NOT_RUNNING:{ LogError("AUDCLNT_E_SERVICE_NOT_RUNNING");break;}
            case E_POINTER:{ LogError("E_POINTER");break;}
            case E_INVALIDARG:{ LogError("E_INVALIDARG");break;}
            case E_OUTOFMEMORY:{ LogError("E_OUTOFMEMORY");break;}
        }
        LogError("IAudioClient Initialize");
        return audio;
    }

    audio.audioRenderClient;
    result = audio.audioClient->lpVtbl->GetService(
        audio.audioClient, &IID_IAudioRenderClient, (void**)&audio.audioRenderClient 
    );

    if(result != S_OK)
    {
        LogError("IAudioClient GetService");
        return audio;
    }

    audio.audioClient->lpVtbl->GetBufferSize(audio.audioClient, &audio.bufferFrameCount);

    audio.bufferDuration = 
        (f64)REFERENCE_TIMES_PER_SEC * audio.bufferFrameCount / audio.samplesPerSecond;

    result = audio.audioClient->lpVtbl->Start(audio.audioClient);

    if(result != S_OK)
    {
        LogError("IAudioClient Start");
        return audio;
    }

    LogSuccess("WASAPI Initialized");

    return audio;
}
