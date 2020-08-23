#define OS_INTERFACE_IMPLEMENTATION
#include "win32_platform_executable.c"
#include "math_library.h"
#include "examples.cpp"

// Called on the start of the app
external void Initialize(operating_system_interface *operatingSystemInterface)
{
    // NOTE: dll has a global os pointer which simplifies the interface 
    // because we dont have to pass the os pointer around to everything
    AttachOS(operatingSystemInterface);
    
    if(operatingSystemInterface->currentRenderer == RENDERER_OPENGL)
    {
        // NOTE: from opengl_procedures.include
        OpenGLLoadProcedures(GetOS()->OpenGLLoadProcedures);
        
        // NOTE: generic opengl triangle example
        OpenGLTriangleSetup();
    }
    
}

// Called on every frame
external void Update(operating_system_interface *operatingSystemInterface)
{
    operating_system_interface *os = GetOS();
    if(IsKeyDown(KEY_ESC)) os->Quit();
    
    //AudioGenerateSineWave(os->audioBuffer, os->requestedSamples);
    
    // NOTE: Draw
    if(os->currentRenderer == RENDERER_OPENGL)
    {
        glClearColor(0, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    else if(os->currentRenderer == RENDERER_SOFTWARE)
    {
        RenderRectangle(&os->graphicsBuffer, 0, 0, (f32)os->graphicsBuffer.size.x,
                        (f32)os->graphicsBuffer.size.y, {0.f, 0.f, 0.f, 0.f});
        RenderRectangle(&os->graphicsBuffer, 0, 0, 100, 100, {1.0f, 0, 0, 1.0f});
    }
}

// Called when you recomplile while the app is running
external void HotReload(operating_system_interface *operatingSystemInterface)
{
    // NOTE: we need to call those on every reload because dll loses all memory
    // when we reload so the global variables get invalidated
    AttachOS(operatingSystemInterface);
    
    if(operatingSystemInterface->currentRenderer == RENDERER_OPENGL)
        OpenGLLoadProcedures(GetOS()->OpenGLLoadProcedures);
}

// Called when you recomplile while the app is running
external void HotUnload(operating_system_interface *operatingSystemInterface)
{
    LogInfo("HotUnload");
}