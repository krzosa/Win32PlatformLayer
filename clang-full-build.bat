@echo off

mkdir bin
cd bin

set EXE_NAME=-OUT:"app.exe"
set DLL_NAME=-OUT:"app_code.dll"

set PLATFORM_FILE=win32_platform_executable
set MAIN_FILE=main

set EXPORTED_FUNCTIONS=/EXPORT:Update /EXPORT:Initialize /EXPORT:HotReload /EXPORT:HotUnload

clang -c -DWIN32_EXE ..\src\%PLATFORM_FILE%.c
link %PLATFORM_FILE%.o %EXE_NAME% user32.lib gdi32.lib opengl32.lib winmm.lib msvcrt.lib msvcprt.lib 

clang -c ..\src\%MAIN_FILE%.c
link -dll %MAIN_FILE%.o %DLL_NAME% opengl32.lib msvcrt.lib %EXPORTED_FUNCTIONS%

cd ..
