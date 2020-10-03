#define OS_INTERFACE_IMPLEMENTATION
#include "win32_platform_executable.c"
#include "supplementary/examples.cpp"


// Called on the start of the app
external void Initialize(operating_system_interface *operatingSystemInterface)
{
    // NOTE: dll has a global os pointer which simplifies the interface 
    // because we dont have to pass the os pointer around to everything
    OSAttach(operatingSystemInterface);
    
    if(operatingSystemInterface->currentRenderer == RENDERER_OPENGL)
    {
        // NOTE: from opengl_procedures.include
        OpenGLLoadProcedures(OSGet()->OpenGLLoadProcedures);
        
        // NOTE: generic opengl triangle example
        OpenGLTriangleSetup();
    }
    else if(operatingSystemInterface->currentRenderer == RENDERER_SOFTWARE) 
    {
        operatingSystemInterface->targetFramesPerSecond = 30;
    }
    
}

// Called on every frame
external void Update(operating_system_interface *operatingSystemInterface)
{
    operating_system_interface *os = OSGet();
    
    if(KeyCheckIfDown(KEY_ESC)) os->Quit();
    
    // AudioGenerateSineWave(os->audioBuffer, os->requestedSamples);
    
    // NOTE: Draw
    if(os->currentRenderer == RENDERER_OPENGL)
    {
        glClearColor(0, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    else if(os->currentRenderer == RENDERER_SOFTWARE)
    {
        DrawRectangle(100, 100, 400, 400, {200, 200, 0, 255});
    }
}

// Called when you recomplile while the app is running
external void HotReload(operating_system_interface *operatingSystemInterface)
{
    // NOTE: we need to call those on every reload because dll loses all memory
    // when we reload so the global variables get invalidated
    OSAttach(operatingSystemInterface);
    
    if(operatingSystemInterface->currentRenderer == RENDERER_OPENGL)
        OpenGLLoadProcedures(OSGet()->OpenGLLoadProcedures);
}

// Called when you recomplile while the app is running
external void HotUnload(operating_system_interface *operatingSystemInterface)
{
    LogInfo("HotUnload");
}