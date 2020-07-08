#include "shared_language_layer.h"
#include "shared_operating_system_interface.h"
#include "operating_system_interface.c"
#include <math.h>

#include "opengl.h"
#include "opengl.c"

internal void
AudioGenerateSineWave(void *audioBuffer, i32 sampleCount, i32 wavePeriod)
{
    #define MATH_PI 3.14159265f
    local_scoped_global f32 tSine;

    i16 *sample = (i16 *)audioBuffer;
    for(i32 i = 0; i != sampleCount; i++)
    {
        f32 sineValue = sinf(tSine);
        i16 sampleValue = (i16)(sineValue * 6000);
        *sample++ = sampleValue;
        *sample++ = sampleValue;

        tSine += 2 * MATH_PI * (f32)1.0f / (f32)wavePeriod;
    }
}

void Initialize(operating_system_interface *operatingSystemInterface)
{
    os = operatingSystemInterface;
    LogSuccess("INIT Operating system attached");
    OpenGLFunctionsLoad(os->OpenGLFunctionLoad);
    OpenGLTriangleSetup();
}
void Update(operating_system_interface *operatingSystemInterface)
{
    glClearColor(0, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // NOTE: Sine wave controlled by W Key and right controller stick
    i32 toneHz = 261 + (i32)(os->userInput.controller[0].rightStickX * 100);
    if(IsKeyDown(KEY_W)) toneHz = 350;
    i32 wavePeriod = (48000 / toneHz);
    AudioGenerateSineWave(os->audioBuffer, os->requestedSamples, wavePeriod);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    if(IsKeyDown(KEY_ESC)) os->Quit();
}
void HotReload(operating_system_interface *operatingSystemInterface)
{
    os = operatingSystemInterface;
    LogSuccess("HOT RELOAD Operating system attached");
}