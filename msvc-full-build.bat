@echo off
mkdir bin

set EXE_NAME=/Fe: "app"
set DLL_NAME=/Fe: "app_code"
set EXPORTED_FUNCTIONS=/EXPORT:Update /EXPORT:Initialize /EXPORT:HotReload /EXPORT:HotUnload

cd bin 
cl ..\src\dll_main.c %DLL_NAME% /LD /Zi /nologo /link opengl32.lib %EXPORTED_FUNCTIONS%
cl ..\src\win32_platform\win32_main.c %EXE_NAME% /Zi /nologo user32.lib gdi32.lib opengl32.lib winmm.lib
cd ..

REM /MTd