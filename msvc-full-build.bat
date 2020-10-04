@echo off
mkdir bin
call utilities\\upgrade_cmd64

set EXE_NAME=-Fe: "app"
set DLL_NAME=-Fe: "app_code"
set EXPORTED_FUNCTIONS=-EXPORT:Update -EXPORT:Initialize -EXPORT:HotReload -EXPORT:HotUnload

cd bin 
cl ..\source_code\main.cpp %DLL_NAME% -LD -Zi -W3 -nologo -link opengl32.lib %EXPORTED_FUNCTIONS%
cl ..\source_code\win32_platform_executable.c %EXE_NAME% -nologo -Zi -W3 -DWIN32_EXE user32.lib gdi32.lib opengl32.lib winmm.lib
cd ..

REM -MTd