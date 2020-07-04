#include "shared_custom.h"
#include "shared_operating_system_interface.h"
#include "operating_system_interface.c"

#define ConsoleLog(text, ...) os->log(text, __VA_ARGS__)
#define ConsoleLogExtra(prepend, text, ...) os->logExtra(prepend, text, __VA_ARGS__)

#include "opengl.h"
#include "opengl.c"


void Initialize(operating_system_interface *operatingSystemInterface)
{
    os = operatingSystemInterface;
    OpenGLFunctionsLoad(os->OpenGLFunctionLoad);
    OpenGLTriangleSetup();
    
}
bool32 Update(operating_system_interface *operatingSystemInterface)
{
    glClearColor(0, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if(IsKeyDown(KEY_W)) Log("W\n");
    if(IsKeyPressedOnce(KEY_ESC)) return 0;
    if(IsKeyUnpressedOnce(KEY_A)) Log("A\n");
    if(IsButtonDown(BUTTON_UP)) Log("A\n");
    if(IsButtonPressedOnce(BUTTON_DOWN)) Log("FF\n");
    if(IsButtonUnpressedOnce(BUTTON_LEFT)) Log("A\n");

    glDrawArrays(GL_TRIANGLES, 0, 3);
    return 1;
}
void HotReload(operating_system_interface *operatingSystemInterface)
{
    os = operatingSystemInterface;
}