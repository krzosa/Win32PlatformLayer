// TODO: test going up to direct sound 8
internal void Win32InitDSound(HWND Window, int32_t samplesPerSecond, int32_t bufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary)
    {
        direct_sound_create* directSoundCreateObject = 
            (direct_sound_create*)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        /* NOTE(Kkrz): first buffer is for setting cooperative level and general settings
                        we write to the second buffer only */
        LPDIRECTSOUND DirectSound;
        if(directSoundCreateObject && SUCCEEDED(directSoundCreateObject(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            {
                WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
                WaveFormat.nChannels = 2;
                WaveFormat.nSamplesPerSec = samplesPerSecond;
                WaveFormat.wBitsPerSample = 16;
                WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
                WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
                WaveFormat.cbSize = 0;
            }
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC bufferDescription = {};
                {
                    bufferDescription.dwSize = sizeof(bufferDescription);
                    bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                }

                LPDIRECTSOUNDBUFFER primaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
                {
                    HRESULT Error = primaryBuffer->SetFormat(&WaveFormat);
                    if(SUCCEEDED(Error))
                    {
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                    else
                    {
                        // TODO: failed to set format for primary buffer
                    }
                }
                else
                {
                    // TODO: failed to create sound buffer 
                }
            }
            else
            {
                // TODO: failed to set cooperative level
            }
            DSBUFFERDESC BufferDescription = {};
            {
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = 0;
                BufferDescription.dwBufferBytes = bufferSize;
                BufferDescription.lpwfxFormat = &WaveFormat;
            }
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if(SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer created successfully.\n");
            }
        }
        else
        {
            // TODO: failed to create sound object
        }
    }
    else
    {
        // TODO: failed to load dll
    }
}