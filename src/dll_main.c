#include "shared_language_layer.h"
#include "shared_string.c"
#include "shared_operating_system_interface.h"
#include "operating_system_interface.c"

#include "opengl.h"
#include "examples.c"

// Called on the start of the app
void Initialize(operating_system_interface *operatingSystemInterface)
{
    // NOTE: dll has a global os pointer which simplifies the interface 
    // because we dont have to pass the os pointer around to everything
    os = operatingSystemInterface;

    // NOTE: from opengl_procedures.include
    OpenGLLoadProcedures(os->OpenGLLoadProcedures);

    // NOTE: generic opengl triangle example
    OpenGLTriangleSetup();

}

// Called on every frame
void Update(operating_system_interface *operatingSystemInterface)
{
    if(IsKeyDown(KEY_ESC)) os->Quit();

    AudioGenerateSineWave(os->audioBuffer, os->requestedSamples);

    // NOTE: Draw
    {
        glClearColor(0, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}

// Called when you recomplile while the app is running
void HotReload(operating_system_interface *operatingSystemInterface)
{
    // NOTE: we need to call those on every reload because dll loses all memory
    // when we reload so the global variables get invalidated
    os = operatingSystemInterface;
    OpenGLLoadProcedures(os->OpenGLLoadProcedures);
}

// Called when you recomplile while the app is running
void HotUnload(operating_system_interface *operatingSystemInterface)
{
    LogInfo("HotUnload");
}