@echo off
mkdir bin
call shell64
call ..\..\utilities\upgrade_cmd64

set EXE_NAME=/Fe: "app"
set DLL_NAME=/Fe: "app_code"
set EXPORTED_FUNCTIONS=/EXPORT:UPDATE /EXPORT:INIT /EXPORT:RELOAD /EXPORT:UNLOAD

cd bin 

cl ..\main.cpp %DLL_NAME% /I ..\.. -FC  /LD /Zi /nologo /link opengl32.lib %EXPORTED_FUNCTIONS%
cl ..\..\win32_platform_executable.c %EXE_NAME% -FC  /Zi /W3 /DWIN32_EXE /nologo user32.lib gdi32.lib opengl32.lib winmm.lib

cd ..

REM /MTd