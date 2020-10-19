#define OS_INTERFACE_IMPLEMENTATION
#include "win32_platform_executable.c"
#include "supplementary/math_library.h"
#include "supplementary/memory_storage_library.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "opengl_renderer.cpp"

struct game_state
{
    
};

// Called on the start of the app
external void Initialize(operating_system_interface *os)
{
    // NOTE: dll has a global os pointer which simplifies the interface 
    // because we dont have to pass the os pointer around to everything
    OSAttach(os);
    
    // NOTE: generic opengl triangle example
    // OpenGLTriangleSetup();
    StorageReset(&os->pernamentStorage);
    opengl_renderer *gl = StoragePushStruct(&os->pernamentStorage, 
                                            opengl_renderer);
    
    OpenGLRendererInitialize(gl);
    OpenGLRendererAttach(gl);
}

// Called on every frame
external void Update(operating_system_interface *os)
{
    if(KeyCheckIfDown(KEY_ESC)) os->Quit();
    StorageReset(&os->pernamentStorage);
    opengl_renderer *gl = StoragePushStruct(&os->pernamentStorage, 
                                            opengl_renderer);
    
    
    StorageReset(&os->temporaryStorage);
    if(KeyCheckIfDownOnce(KEY_F1))
    {
        os->WindowSetTransparency(40);
        os->WindowAlwaysOnTop();
    }
    if(KeyCheckIfDownOnce(KEY_F2))
    {
        os->WindowSetTransparency(255);
        os->WindowNotAlwaysOnTop();
    }
    
    m4x4 projectionMatrix = OrtographicProjectionMatrix(0, 1280, 0, 720, 0, 500);
    ShaderUniform(gl->basicShader, "viewProjectionMatrix", projectionMatrix);
    
    DrawBegin();
    {
        DrawRectangle({200, 200, 300, 300}, {0,0.7f,0.5f,1.f});
    }
    DrawEnd();
}

// Called when you recomplile while the app is running
external void HotReload(operating_system_interface *os)
{
    // NOTE: we need to call those on every reload because dll loses all memory
    // when we reload so the global variables get invalidated
    OSAttach(os);
    
    StorageReset(&os->pernamentStorage);
    opengl_renderer *gl = StoragePushStruct(&os->pernamentStorage, 
                                            opengl_renderer);
    OpenGLRendererAttach(gl);
    OpenGLRendererInitialize(gl);
}

// Called when you recomplile while the app is running
external void HotUnload(operating_system_interface *os)
{
    LogInfo("HotUnload");
    OpenGLRendererDestroy();
}