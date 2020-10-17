#define OS_INTERFACE_IMPLEMENTATION
#include "win32_platform_executable.c"
#include "supplementary/math_library.h"
#include "supplementary/memory_storage_library.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "opengl_renderer.cpp"


// Called on the start of the app
external void Initialize(operating_system_interface *os)
{
    // NOTE: dll has a global os pointer which simplifies the interface 
    // because we dont have to pass the os pointer around to everything
    OSAttach(os);

    // NOTE: generic opengl triangle example
    // OpenGLTriangleSetup();
    StorageReset(&os->pernamentStorage);
    gl = PernamentPushStruct(opengl_renderer, os);
    OpenGLRendererInitialize(gl);
}

// Called on every frame
external void Update(operating_system_interface *os)
{
    if(KeyCheckIfDown(KEY_ESC)) os->Quit();
    
    StorageReset(&os->temporaryStorage);
    if(KeyCheckIfDownOnce(KEY_F1))
    {
        os->WindowSetTransparency(70);
        os->WindowAlwaysOnTop();
    }
    if(KeyCheckIfDownOnce(KEY_F2))
    {
        os->WindowSetTransparency(255);
        os->WindowNotAlwaysOnTop();
    }
    
    static f32 offsetX;
    static f32 offsetY;
    f32 offsetValue = 10.f;
    if(KeyCheckIfDown(KEY_D)) offsetX += offsetValue;
    if(KeyCheckIfDown(KEY_A)) offsetX -= offsetValue;
    if(KeyCheckIfDown(KEY_S)) offsetY -= offsetValue;
    if(KeyCheckIfDown(KEY_W)) offsetY += offsetValue;
    
    
    gl->camera.zoom += (f32)(MouseGetWheel() / 8.f);
    gl->camera.position.x = offsetX;
    gl->camera.position.y = offsetY;
    
    m4x4 cameraMatrix = CameraMatrix(gl->camera);
    m4x4 projectionMatrix = OrtographicProjectionMatrix(0, 1280, 0, 720, 0, 500);
    
    m4x4 viewProjectionMatrix = projectionMatrix * cameraMatrix;
    ShaderUniform(gl->basicShader, "viewProjectionMatrix", viewProjectionMatrix);
    
    BeginDrawing();
    {
        PushQuad(gl, QuadTextured({100, 300}, {400, 400}, 1));
        PushQuad(gl, QuadTextured({300, 300}, {100, 400}, 2));
        PushQuad(gl, QuadTextured({500, 500}, {100, 400}, 2));
        PushQuad(gl, QuadColored({500, 100}, {400, 400}, {0,0.7f,0.5f,1.f}));
        PushQuad(gl, QuadColored({2000, 2000}, {400, 400}, {0,0.7f,0.5f,1.f}));
    }
    EndDrawing();
}

// Called when you recomplile while the app is running
external void HotReload(operating_system_interface *os)
{
    // NOTE: we need to call those on every reload because dll loses all memory
    // when we reload so the global variables get invalidated
    OSAttach(os);
    gl = PernamentPushStruct(opengl_renderer, os);
    OpenGLRendererInitialize(gl);
}

// Called when you recomplile while the app is running
external void HotUnload(operating_system_interface *os)
{
    LogInfo("HotUnload");
    OpenGLRendererDestroy(gl);
}