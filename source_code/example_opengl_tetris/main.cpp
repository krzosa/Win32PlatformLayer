#define OS_INTERFACE_IMPLEMENTATION
#include "win32_platform_executable.c"
#include <malloc.h>
#include "krz/krz_string.c"
#include "krz/krz_arena.h"
#include "krz/krz_math.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "opengl_renderer.h"
#include "assets.c"
#include "opengl_renderer.cpp"

struct GameState
{
    MemoryArena openglArena;
    OpenGLRenderer *renderer;
    
    Texture2D wall;
    Texture2D face;
    Font font;
};

// Called on the start of the app
void Initialize(OperatingSystemInterface *os)
{
    GameState *game = (GameState *)os->memory;
    void *arenaMemory = (void *)(game + 1);
    ArenaInitialize(&game->openglArena, arenaMemory, os->memorySize - sizeof(GameState));
    
    HotReload(os);
}

// Called on every frame
void Update(OperatingSystemInterface *os)
{
    GameState *game = (GameState *)os->memory;
    
    if(KeyTap(KEY_F1))
    {
        os->WindowSetTransparency(100);
        os->WindowAlwaysOnTop();
    }
    if(KeyTap(KEY_F2))
    {
        os->WindowSetTransparency(255);
        os->WindowNotAlwaysOnTop();
    }
    
    DrawBegin();
    {
        for(i32 i = 0; i < 100; i++)
        {
            i32 x = 0;
            i32 y = 0;
            if(i % 2)
                x = i;
            else
                y = i;
            
            DrawRectangle({0 + (f32)x, 0 + (f32)y, 40, 40}, {0,0.7f,0.5f,1.f});
        }
        
        DrawSprite({90, 40, 50, 50}, game->wall);
        DrawSprite({40, 40, 50, 50}, game->face);
        DrawSprite({40, 40, 150, 150}, {game->font.id});
        DrawText('a' - 33, {0,0}, &game->font);
    }
    DrawEnd();
    
    os->ScreenRefresh();
}

// Called when you recomplile while the app is running
void HotReload(OperatingSystemInterface *os)
{
    GameState *game = (GameState *)os->memory;
    game->renderer = ArenaPushStruct(&game->openglArena, OpenGLRenderer);
    OpenGLRendererInit(&game->openglArena, game->renderer, 320, 180);
    game->font = FontLoad(&game->openglArena, "/data/LiberationMono-Regular.ttf");
    game->face = TextureCreate("/data/awesomeface.png");
    game->wall = TextureCreate("/data/wall.jpg");
}

// Called when you recomplile while the app is running
void HotUnload(OperatingSystemInterface *os)
{
    GameState *game = (GameState *)os->memory;
    OpenGLRendererDestroy();
    ArenaZeroTheWholeThing(&game->openglArena);
}