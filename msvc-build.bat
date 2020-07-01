@echo off

set EXE_NAME=/Fe: "app"
set DLL_NAME=/Fe: "app_code"
set EXPORTED_FUNCTIONS=/EXPORT:Update /EXPORT:Initialize /EXPORT:HotReload

cd bin 
ctime -begin compileTime
cl ..\src\update_loop.c %DLL_NAME% /LD /Zi /nologo /link %EXPORTED_FUNCTIONS%
cl ..\src\win32_main.c  %EXE_NAME% /Zi /nologo user32.lib gdi32.lib opengl32.lib winmm.lib
ctime -end compileTime
cd ..