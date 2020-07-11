#include <dsound.h>

// NOTE: we will load this function dynamically from a dll
typedef HRESULT WINAPI direct_sound_create(LPGUID lpGuid, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

typedef struct win32_audio_data
{
    LPDIRECTSOUNDBUFFER audioBuffer;
    u32 currentPositionInBuffer;
    i32 samplesPerSecond;
    i32 numberOfChannels;
    i32 bytesPerSample;
    i32 bufferSize;
    i32 audioLatency;
    bool32 isPlaying;
} win32_audio_data;

typedef struct to_lock
{
    DWORD numberOfBytesToLock;
    DWORD byteToLock;
    bool32 soundIsValid;
} to_lock;

internal LPDIRECTSOUNDBUFFER
Win32AudioInitialize(HWND window, i32 samplesPerSecond, i32 bufferSize)
{
    LPDIRECTSOUNDBUFFER secondaryBuffer = 0;
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if (DSoundLibrary)
    {
        direct_sound_create *directSoundCreateObject =
            (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        Assert(directSoundCreateObject);

        // NOTE: first buffer is for setting cooperative level and general settings
        //       we write to the second buffer only
        LPDIRECTSOUND directSound;
        if (directSoundCreateObject && SUCCEEDED(directSoundCreateObject(0, &directSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {0};
            {
                WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
                WaveFormat.nChannels = 2;
                WaveFormat.nSamplesPerSec = samplesPerSecond;
                WaveFormat.wBitsPerSample = 16;
                WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
                WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
                WaveFormat.cbSize = 0;
            }
            if (SUCCEEDED(directSound->lpVtbl->SetCooperativeLevel(
                    directSound, window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC bufferDescription = {0};
                {
                    bufferDescription.dwSize = sizeof(bufferDescription);
                    bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                }

                LPDIRECTSOUNDBUFFER primaryBuffer;
                if (SUCCEEDED(directSound->lpVtbl->CreateSoundBuffer(
                        directSound, &bufferDescription, &primaryBuffer, 0)))
                {
                    HRESULT Error = primaryBuffer->lpVtbl->SetFormat(primaryBuffer, &WaveFormat);
                    if (SUCCEEDED(Error))
                    {
                        LogSuccess("Primary buffer format was set");
                    }
                    else
                    {
                        LogError("failed to set format for primary buffer");
                    }
                }
                else
                {
                    LogError("failed to create sound buffer");
                }
            }
            else
            {
                LogError("failed to set cooperative level");
            }
            DSBUFFERDESC bufferDescriptor = {0};
            {
                bufferDescriptor.dwSize = sizeof(bufferDescriptor);
                bufferDescriptor.dwFlags = 0;
                bufferDescriptor.dwBufferBytes = bufferSize;
                bufferDescriptor.lpwfxFormat = &WaveFormat;
            }
            HRESULT Error = directSound->lpVtbl->CreateSoundBuffer(
                directSound, &bufferDescriptor, &secondaryBuffer, 0
            );
            if (SUCCEEDED(Error))
            {
                LogSuccess("Secondary buffer created");
            }
        }
        else
        {
            LogError("Direct sound failed to create sound object");
        }
    }
    else
    {
        LogError("Direct sound failed to load dll");
    }

    return secondaryBuffer;
}

internal void 
Win32AudioBufferFill(win32_audio_data *audioData, void *bufferToPlay, DWORD byteToLock, DWORD numberOfBytesToLock)
{
    VOID *region1 = 0;
    VOID *region2 = 0;
    DWORD region1Size = 0;
    DWORD region2Size = 0;
    static f32 tSine;
    LPDIRECTSOUNDBUFFER audioBuffer = audioData->audioBuffer;

    if(SUCCEEDED(audioBuffer->lpVtbl->Lock(audioBuffer, 
                                            byteToLock, numberOfBytesToLock, 
                                            &region1, &region1Size, 
                                            &region2, &region2Size, 0)))
    {
        i16 *sampleIn = (i16 *)bufferToPlay;
        i32 region1SampleCount = region1Size / audioData->bytesPerSample;
        i32 region2SampleCount = region2Size / audioData->bytesPerSample;

        i16 *sampleOut = (i16 *)region1;
        for(i32 i = 0; i != region1SampleCount; i++)
        {
            *sampleOut++ = *sampleIn++;
            *sampleOut++ = *sampleIn++;
        }

        sampleOut = (i16 *)region2;
        for(i32 i = 0; i != region2SampleCount; i++)
        {
            *sampleOut++ = *sampleIn++;    
            *sampleOut++ = *sampleIn++;
        }

        audioBuffer->lpVtbl->Unlock(audioBuffer, 
                                    region1, region1Size, 
                                    region2, region2Size);
    } 
}

internal void 
Win32AudioBufferZeroClear(win32_audio_data *audioData)
{
    VOID *region1 = 0;
    VOID *region2 = 0;
    DWORD region1Size = 0;
    DWORD region2Size = 0;
    LPDIRECTSOUNDBUFFER audioBuffer = audioData->audioBuffer;

    if(SUCCEEDED(audioBuffer->lpVtbl->Lock(audioBuffer, 
                                            0, audioData->bufferSize, 
                                            &region1, &region1Size, 
                                            &region2, &region2Size, 0)))
    {
        i8 *sample = (i8 *)region1;
        for(i32 i = 0; i != region1Size; i++)
        {
            *sample++ = 0;
        }

        sample = (i8 *)region2;
        for(i32 i = 0; i != region2Size; i++)
        {
            *sample++ = 0;
        }

        audioBuffer->lpVtbl->Unlock(audioBuffer, 
                                    region1, region1Size, 
                                    region2, region2Size);
    } 
}

internal to_lock
Win32FigureItOut(win32_audio_data *audioData)
{
    // NOTE: Figure out how much sound we want to request and figure out
    //       our position in the direct sound buffer
    to_lock toLock = {0};
    DWORD playCursor = 0;
    DWORD writeCursor = 0;
    toLock.byteToLock = audioData->currentPositionInBuffer;
    if(SUCCEEDED(audioData->audioBuffer->lpVtbl->GetCurrentPosition(audioData->audioBuffer, 
                                                            &playCursor, &writeCursor)))
    {
        i32 samplesOfLatency = (audioData->audioLatency * audioData->bytesPerSample);
        DWORD targetCursor = (playCursor + samplesOfLatency) % audioData->bufferSize;
        if(toLock.byteToLock > targetCursor)
        {
            toLock.numberOfBytesToLock = audioData->bufferSize - toLock.byteToLock;
            toLock.numberOfBytesToLock += targetCursor;
        }
        else toLock.numberOfBytesToLock = targetCursor - toLock.byteToLock;

        // NOTE: calculate next position in the audio buffer
        audioData->currentPositionInBuffer += toLock.numberOfBytesToLock;
        audioData->currentPositionInBuffer %= audioData->bufferSize;

        toLock.soundIsValid = true;
    }

    return toLock;
}

internal void
Win32PlayAudio(win32_audio_data *audioData)
{
    if(!audioData->isPlaying)
    {
        if(!SUCCEEDED(audioData->audioBuffer->lpVtbl->Play(audioData->audioBuffer, 0, 0, 
                                                        DSBPLAY_LOOPING))) 
        {
            LogError("AudioBuffer Play");
            Assert(0);
        }
        audioData->isPlaying = !audioData->isPlaying;
    }
}

// if(toLock.soundIsValid)
// {
//     Win32AudioBufferFill(&audioData, os->audioBuffer, toLock.byteToLock, toLock.numberOfBytesToLock);
// }

// audioData.audioBuffer = Win32AudioInitialize(windowHandle, 
//                                                  audioData.samplesPerSecond, 
//                                                  audioData.bufferSize);