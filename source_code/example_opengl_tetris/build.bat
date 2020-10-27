@echo off
mkdir bin
call ..\..\utilities\upgrade_cmd64

set EXE_NAME=/Fe: "app"
set DLL_NAME=/Fe: "app_code"
set EXPORTED_FUNCTIONS=/EXPORT:Update /EXPORT:Initialize /EXPORT:HotReload /EXPORT:HotUnload

cd bin 

cl ..\main.cpp %DLL_NAME% /I ..\.. /LD /Zi /nologo /link opengl32.lib %EXPORTED_FUNCTIONS%
cl ..\..\win32_platform_executable.c %EXE_NAME% /Zi /W3 /DWIN32_EXE /nologo user32.lib gdi32.lib opengl32.lib winmm.lib

cd ..

REM /MTd