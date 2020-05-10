@echo off

cd ..\build 
cl /Zi ..\src\win32_main.cpp user32.lib gdi32.lib 
cd ..\src