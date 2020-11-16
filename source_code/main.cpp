#define OS_INTERFACE_IMPLEMENTATION
#include "win32_platform_executable.c"

// Called on the start of the app
void Initialize(operating_system_interface *os)
{
    LogInfo("Initialize");
}

// Called on every frame
void Update(operating_system_interface *os)
{
    glClearColor(0, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

// Called when you recomplile while the app is running
void HotReload(operating_system_interface *os)
{
    LogInfo("HotReload");
}

// Called when you recomplile while the app is running
void HotUnload(operating_system_interface *os)
{
    LogInfo("HotUnload");
}

// #include <math.h>
// internal void
// AudioGenerateSineWave(void *audioBuffer, i32 sampleCount)
// {
//     // NOTE: Sine wave controlled by W Key and right controller stick
//     i32 toneHz = 261 + (i32)(OSGet()->userInput.controller[0].rightStickX * 100);
//     if(KeyCheckIfDown(KEY_W)) toneHz = 350;
//     i32 wavePeriod = (48000 / toneHz);


// #define MATH_PI 3.14159265f
//     static f32 tSine;

//     i16 *sample = (i16 *)audioBuffer;
//     for(i32 i = 0; i != sampleCount; i++)
//     {
//         f32 sineValue = sinf(tSine);
//         i16 sampleValue = (i16)(sineValue * 3000);
//         *sample++ = sampleValue;
//         *sample++ = sampleValue;

//         tSine += 2 * MATH_PI * (f32)1.0f / (f32)wavePeriod;
//     }
// }
// AudioGenerateSineWave(os->audioBuffer, os->requestedSamples);