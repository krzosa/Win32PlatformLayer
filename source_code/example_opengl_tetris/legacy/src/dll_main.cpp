#define OS_INTERFACE_IMPLEMENTATION
#include "../../win32_platform_executable.c"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "examples.cpp"

// Called when you recomplile while the app is running
extern "C" void HotReload(operating_system_interface *operatingSystemInterface)
{
    // NOTE: we need to call those on every reload because dll loses all memory
    // when we reload so the global variables get invalidated
    os = operatingSystemInterface;
    OpenGLLoadProcedures(os->OpenGLLoadProcedures);
    // NOTE ON ERRORS If I get an error that appears only on first app run
    // then the bug is caused because of some hot reload meme
    // like the fact that all pointers get purged and stuff


    StorageReset(&os->pernamentStorage);
    gl = PernamentPushStruct(opengl_context, os);
    OpenGLContextInitialize(gl);
}

// Called on the start of the app
extern "C" void Initialize(operating_system_interface *operatingSystemInterface)
{
    os = operatingSystemInterface;
    OpenGLLoadProcedures(os->OpenGLLoadProcedures);
    
    gl = PernamentPushStruct(opengl_context, os);
    OpenGLContextInitialize(gl);
}

// Called on every frame
extern "C" void Update(operating_system_interface *operatingSystemInterface)
{
    StorageReset(&os->temporaryStorage);
    if(IsKeyDown(KEY_ESC)) os->Quit();
    if(IsKeyPressedOnce(KEY_F1))
    {
        os->WindowSetTransparency(70);
        os->WindowAlwaysOnTop();
    }
    if(IsKeyPressedOnce(KEY_F2))
    {
        os->WindowSetTransparency(255);
        os->WindowNotAlwaysOnTop();
    }
    
    static f32 offsetX;
    static f32 offsetY;
    f32 offsetValue = 10.f;
    if(IsKeyDown(KEY_D)) offsetX += offsetValue;
    if(IsKeyDown(KEY_A)) offsetX -= offsetValue;
    if(IsKeyDown(KEY_S)) offsetY -= offsetValue;
    if(IsKeyDown(KEY_W)) offsetY += offsetValue;
    
    
    gl->camera.zoom += (f32)(MouseWheelStatus() / 8.f);
    gl->camera.position.x = offsetX;
    gl->camera.position.y = offsetY;
    
    m4x4 cameraMatrix = CameraMatrix(gl->camera);
    m4x4 projectionMatrix = OrtographicProjectionMatrix(0, 1280, 0, 720, 0, 500);
    
    m4x4 viewProjectionMatrix = projectionMatrix * cameraMatrix;
    gl->basicShader.Uniform("viewProjectionMatrix", viewProjectionMatrix);
    
    DrawBegin();
    {
        PushQuad(gl, QuadTextured({100, 300}, {400, 400}, 1));
        PushQuad(gl, QuadTextured({300, 300}, {100, 400}, 2));
        PushQuad(gl, QuadTextured({500, 500}, {100, 400}, 2));
        PushQuad(gl, QuadColored({500, 100}, {400, 400}, {0,0.7f,0.5f,1.f}));
        PushQuad(gl, QuadColored({2000, 2000}, {400, 400}, {0,0.7f,0.5f,1.f}));
    }
    DrawEnd();
}


// Called when you recomplile while the app is running
extern "C" void HotUnload(operating_system_interface *operatingSystemInterface)
{
    LogInfo("HotUnload");
    OpenGLContextDestroy(gl);
}