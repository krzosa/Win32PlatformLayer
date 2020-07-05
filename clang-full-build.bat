cd bin

set EXPORTED_FUNCTIONS=/EXPORT:Update /EXPORT:Initialize /EXPORT:HotReload

clang -c ..\src\win32_platform\win32_main.c 
link win32_main.o user32.lib gdi32.lib opengl32.lib winmm.lib msvcrt.lib msvcprt.lib 

clang -c ..\src\dll_main.c
link -dll -OUT:"app_code.dll" dll_main.o opengl32.lib msvcrt.lib %EXPORTED_FUNCTIONS%

cd ..
