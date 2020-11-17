#define OS_INTERFACE_IMPLEMENTATION
#include "win32_platform_executable.c"
#include <malloc.h>
#include "krz/krz_string.c"
#include "krz/krz_arena.h"
#include "krz/krz_math.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "opengl_renderer.cpp"

struct GameState
{
    MemoryArena openglArena;
    OpenGLRenderer *renderer;
};

// Called on the start of the app
void Initialize(OperatingSystemInterface *os)
{
    GameState *game = (GameState *)os->memory;
    void *arenaMemory = (void *)(game + 1);
    ArenaInitialize(&game->openglArena, arenaMemory, os->memorySize - sizeof(GameState));
    
    game->renderer = ArenaPushStruct(&game->openglArena, OpenGLRenderer);
    OpenGLRendererInitialize(game->renderer);
    OpenGLRendererAttach(game->renderer);
}

// Called on every frame
void Update(OperatingSystemInterface *os)
{
    GameState *game = (GameState *)os->memory;
    
    if(KeyTap(KEY_F1))
    {
        os->WindowSetTransparency(40);
        os->WindowAlwaysOnTop();
    }
    if(KeyTap(KEY_F2))
    {
        os->WindowSetTransparency(255);
        os->WindowNotAlwaysOnTop();
    }
    
    M4x4 projectionMatrix = OrtographicProjectionMatrix(0, 320, 0, 180, 0, 1);
    ShaderUniform(game->renderer->basicShader, "viewProjectionMatrix", projectionMatrix);
    
    DrawBegin();
    {
        DrawRectangle({0, 0, 40, 40}, {0,0.7f,0.5f,1.f});
        DrawSprite({40, 40, 50, 50}, 1);
    }
    DrawEnd();
    os->ScreenRefresh();
}

// Called when you recomplile while the app is running
void HotReload(OperatingSystemInterface *os)
{
    GameState *game = (GameState *)os->memory;
    OpenGLRendererAttach(game->renderer);
    OpenGLRendererInitialize(game->renderer);
}

// Called when you recomplile while the app is running
void HotUnload(OperatingSystemInterface *os)
{
    LogInfo("HotUnload");
    OpenGLRendererDestroy();
}