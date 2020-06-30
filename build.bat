@echo off

cd bin 
ctime -begin compileTime
cl /Zi /nologo ..\src\win32_main.c user32.lib gdi32.lib opengl32.lib winmm.lib
ctime -end compileTime
cd ..