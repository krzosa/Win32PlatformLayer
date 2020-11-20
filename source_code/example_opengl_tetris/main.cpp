#define OS_INTERFACE_IMPLEMENTATION
#include "win32_platform_executable.c"
#include <malloc.h>
#include "krz/krz_string.c"
#include "krz/krz_arena.h"
#include "krz/krz_math.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "opengl_renderer.h"
#include "opengl_renderer.cpp"

struct GameState
{
    MemoryArena openglArena;
    OpenGLRenderer *renderer;
    
    Texture2D wall;
    Texture2D face;
};

// Called on the start of the app
void Initialize(OperatingSystemInterface *os)
{
    GameState *game = (GameState *)os->memory;
    void *arenaMemory = (void *)(game + 1);
    ArenaInitialize(&game->openglArena, arenaMemory, os->memorySize - sizeof(GameState));
    
    
    // str8 *path = StringConcatChar(OS->exeDir, "/data");
    // FilePaths *root = os->DirectoryGetFilePaths(path);
    // for(FilePaths *node = root; node; node = node->next)
    // {
    // LogInfo("%s", node->filePath);
    // }
    // os->FilePathsFree(root);
    // void *memory = ArenaPushSize(&game->openglArena, 1000000);
    // 
    // Files files = os->DirectoryReadAllFiles(path, memory, 1000000);
    game->renderer = ArenaPushStruct(&game->openglArena, OpenGLRenderer);
    HotReload(os);
    
    
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
    }
    DrawEnd();
    os->ScreenRefresh();
}

// Called when you recomplile while the app is running
void HotReload(OperatingSystemInterface *os)
{
    GameState *game = (GameState *)os->memory;
    OpenGLRendererInit(&game->openglArena, game->renderer, 320, 180);
    game->face = TextureCreate("/data/awesomeface.png");
    game->wall = TextureCreate("/data/wall.jpg");
}

// Called when you recomplile while the app is running
void HotUnload(OperatingSystemInterface *os)
{
    LogInfo("HotUnload");
    OpenGLRendererDestroy();
}