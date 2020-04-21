@echo off

cd ..\build
cl /Zi ..\src\win32_game.cpp user32.lib
cd ..\src