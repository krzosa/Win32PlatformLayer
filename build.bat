@echo off

cd bin 
ctime -begin compileTime
cl /Zi /nologo ..\src\win32_main.cpp user32.lib gdi32.lib opengl32.lib winmm.lib
ctime -end compileTime
cd ..