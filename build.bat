@echo off

cd build 
ctime -begin aa
cl /Zi /nologo ..\src\win32_main.cpp user32.lib gdi32.lib opengl32.lib
ctime -end aa
cd ..