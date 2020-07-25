#include <objbase.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>

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
CoCreateInstanceFunction *CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
CoInitializeExFunction *CoInitializeExFunctionPointer = CoInitializeExStub;

// NOTE: Number of REFERENCE_TIME units per second
// One unit is equal to 100 nano seconds
#define REF_TIMES_PER_SECOND 10000000

typedef struct win32_audio_data
{
    IMMDevice *device;
    IAudioClient *audioClient;
    IMMDeviceEnumerator *deviceEnum;
    IAudioRenderClient *audioRenderClient;

    u32 samplesPerSecond;
    u32 numberOfChannels;

    // NOTE: one frame is 2 samples (left, right) 32 bits if
    // one sample is equal 16 bit
    u32 bufferFrameCount; 
    u32 latencyFrameCount;
    i32 bitsPerSample;
    REFERENCE_TIME bufferDuration;
    bool32 initialized;
} win32_audio_data;

internal inline u32
AudioBufferSize(win32_audio_data audioData)
{
    return audioData.bufferFrameCount * (audioData.bitsPerSample * 2);
}

// NOTE: Load COM Library functions dynamically, 
//       this way sound is not necessary to run the game
internal void
Win32COMLoad(void)
{
    HMODULE ole32Library = LoadLibraryA("ole32.dll");
    if (ole32Library)
    {
        LogSuccess("COM Ole32.dll Loaded");
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

// NOTE: Bigger number, smaller latency 
internal win32_audio_data
Win32AudioInitialize(i32 samplesPerSecond)
{
    Win32COMLoad();

    win32_audio_data audio = {0};

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

    audio.bitsPerSample = 16;
    audio.numberOfChannels = 2;
    audio.samplesPerSecond = samplesPerSecond;
    WAVEFORMATEX waveFormat = {0};
    {
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = audio.numberOfChannels;
        waveFormat.nSamplesPerSec = audio.samplesPerSecond;
        waveFormat.wBitsPerSample = audio.bitsPerSample;
        waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize = 0;
    }

    REFERENCE_TIME requestedBufferDuration = REF_TIMES_PER_SECOND * 2;

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

    audio.bufferDuration = (REFERENCE_TIME)((f64)REF_TIMES_PER_SECOND *
                            (audio.bufferFrameCount / audio.samplesPerSecond));

    LogInfo("WASAPI Audio buffer frame count: %d", audio.bufferFrameCount);
    LogInfo("WASAPI Audio buffer duration: %lld (10000000 is 1 second)", audio.bufferDuration);

    result = audio.audioClient->lpVtbl->Start(audio.audioClient);
    if(result != S_OK)
    {
        LogError("IAudioClient Start");
        return audio;
    };

    audio.initialized = 1;
    LogSuccess("WASAPI Initialized");

    return audio;
}

internal void
Win32AudioBufferFill(u32 samplesToWrite, i16 *samples, win32_audio_data *output)
{
    if(samplesToWrite)
    {
        BYTE *data = 0;
        DWORD flags = 0;
        
        output->audioRenderClient->lpVtbl->GetBuffer(
            output->audioRenderClient, samplesToWrite, &data
        );
        if(data)
        {
            i16 *destination = (i16 *)data;
            i16 *source = samples;
            for(UINT32 i = 0; i < samplesToWrite; ++i)
            {
                i16 left = (i16)(*source++);
                i16 right = (i16)(*source++);
                *destination++ = left;
                *destination++ = right;
            }
        }
        output->audioRenderClient->lpVtbl->ReleaseBuffer(
            output->audioRenderClient, samplesToWrite, flags
        );
    }
}

internal u32
Win32AudioStatusUpdate(win32_audio_data *audioData, f32 currentFramesPerSecond, f32 latencyMultiplier)
{
    u32 samplesToWrite = 0;
    if(audioData->initialized)
    {
        audioData->latencyFrameCount = (u32)(audioData->samplesPerSecond / 
                                             currentFramesPerSecond * 
                                             latencyMultiplier);
        UINT32 padding;
        if(SUCCEEDED(audioData->audioClient->lpVtbl->GetCurrentPadding(audioData->audioClient, &padding)))
        {
            samplesToWrite = audioData->latencyFrameCount - padding;
            if(samplesToWrite > audioData->latencyFrameCount)
            {
                samplesToWrite = audioData->latencyFrameCount;
            }
        }

        // i32 *buffer = (i32 *)os->audioBuffer;
        // for(u32 i = 0; i < audioData->bufferFrameCount; i++)
        // {
        //     buffer[i] = 0;
        // }
    }

    return samplesToWrite;
}

internal void
Win32WasapiCleanup(win32_audio_data *audio)
{
    if(audio->initialized)
    {
        audio->audioClient->lpVtbl->Stop(audio->audioClient);
        
        audio->deviceEnum->lpVtbl->Release(audio->deviceEnum);
        audio->device->lpVtbl->Release(audio->device);
        audio->audioClient->lpVtbl->Release(audio->audioClient);
        audio->audioRenderClient->lpVtbl->Release(audio->audioRenderClient);
    }
}