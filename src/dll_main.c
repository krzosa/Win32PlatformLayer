#include "shared_language_layer.h"
#include "shared_operating_system_interface.h"
#include "operating_system_interface.c"
#include <math.h>

#include "opengl.h"
#include "opengl.c"

internal void
AudioFillBuffer(void *audioBuffer, i32 sampleCount, i32 wavePeriod)
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
bool32 Update(operating_system_interface *operatingSystemInterface)
{
    glClearColor(0, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    i32 toneHz = 261 + (os->userInput.controller[0].rightStickX * 100);
    i32 wavePeriod = (48000 / toneHz);
    AudioFillBuffer(os->pernamentStorage.memory, os->numberOfSamplesToUpdate, wavePeriod);

    if(IsKeyDown(KEY_W)) Log("W\n");
    if(IsKeyPressedOnce(KEY_ESC)) return 0;
    if(IsKeyUnpressedOnce(KEY_A)) Log("A\n");
    if(IsButtonDown(BUTTON_UP)) Log("A\n");
    if(IsButtonPressedOnce(BUTTON_DOWN)) Log("FF\n");
    if(IsButtonUnpressedOnce(BUTTON_LEFT)) Log("A\n");

    glDrawArrays(GL_TRIANGLES, 0, 3);
    return 1;
}
void HotReload(operating_system_interface *operatingSystemInterface)
{
    os = operatingSystemInterface;
    LogSuccess("HOT RELOAD Operating system attached");
}