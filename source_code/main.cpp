#define OS_INTERFACE_IMPLEMENTATION
#include "win32_platform_executable.c"
#include "supplementary/examples.cpp"


// Called on the start of the app
external void Initialize(operating_system_interface *os)
{
    // NOTE: dll has a global os pointer which simplifies the interface 
    // because we dont have to pass the os pointer around to everything
    OSAttach(os);
    
    // NOTE: generic opengl triangle example
    OpenGLTriangleSetup();
}

// Called on every frame
external void Update(operating_system_interface *os)
{
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
external void HotReload(operating_system_interface *os)
{
    // NOTE: we need to call those on every reload because dll loses all memory
    // when we reload so the global variables get invalidated
    OSAttach(os);
}

// Called when you recomplile while the app is running
external void HotUnload(operating_system_interface *os)
{
    LogInfo("HotUnload");
}