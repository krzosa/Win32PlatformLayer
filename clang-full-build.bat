@echo off

call utilities\\upgrade_cmd64

mkdir bin
cd bin

set EXE_NAME=-OUT:"app.exe"
set DLL_NAME=-OUT:"app_code.dll"

set EXPORTED_FUNCTIONS=/EXPORT:Update /EXPORT:Initialize /EXPORT:HotReload /EXPORT:HotUnload

clang -c -DWIN32_EXE -Wno-write-strings ..\source_code\win32_platform_executable.c
link win32_platform_executable.o %EXE_NAME% user32.lib gdi32.lib opengl32.lib winmm.lib msvcrt.lib msvcprt.lib 

clang -c -Wno-write-strings  ..\source_code\main.cpp
link -dll main.o %DLL_NAME% opengl32.lib msvcrt.lib %EXPORTED_FUNCTIONS%

cd ..
