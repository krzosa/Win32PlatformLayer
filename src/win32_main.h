#pragma once
#include <windows.h>
#include <Xinput.h>
#include <dsound.h>
#include <stdint.h>

#define global_variable static
// localy scoped variable that will persist its value
// when function goes out of scope
#define local_persist static
// reserved for functions that are going to
// be internal to this file, local functions
#define internal static

// Casey Muratori's defines for library loading
// XINPUT
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}

// DIRECTSOUND
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

struct win32_offscreen_buffer
{
    // The BITMAPINFOHEADER structure 
    // contains information about the dimensions 
    // and color format of a DIB. 
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    // this is the value which you want to add to the index
    // if you want to get to the next row when storing 2d thing in 1d
    int pitch;
    // Note: Pixels are always 32bits wide
};

struct window_dimension
{
    int width;
    int height;
};

struct win32_sound_output
{
    int samplesPerSecond;
    int toneHz;
    int16_t toneVolume;
    uint32_t runningSampleIndex;
    int wavePeriod;
    int bytesPerSample;
    int secondaryBufferSize;
    float tSine;
    int latencySampleCount;
};