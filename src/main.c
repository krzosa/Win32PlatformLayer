#define OS_INTERFACE_IMPLEMENTATION
#include "win32_platform_executable.c"
#include "examples.c"

// Called on the start of the app
void Initialize(operating_system_interface *operatingSystemInterface)
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
void Update(operating_system_interface *operatingSystemInterface)
{
    operating_system_interface *os = GetOS();
    if(IsKeyDown(KEY_ESC)) os->Quit();
    
    AudioGenerateSineWave(os->audioBuffer, os->requestedSamples);
    // NOTE: Draw
    if(os->currentRenderer == RENDERER_OPENGL)
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
    AttachOS(operatingSystemInterface);
    
    if(operatingSystemInterface->currentRenderer == RENDERER_OPENGL)
        OpenGLLoadProcedures(GetOS()->OpenGLLoadProcedures);
}

// Called when you recomplile while the app is running
void HotUnload(operating_system_interface *operatingSystemInterface)
{
    LogInfo("HotUnload");
}