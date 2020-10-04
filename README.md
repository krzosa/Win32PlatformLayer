## Overview

Small Windows platform layer(operating system abstraction layer), the purpose of a platform layer is to make an application portable across operating systems, for example if you want to run an application built on top of a Windows based platform layer on Linux instead of Windows then you don't have to rewrite the entire application, you only need to rewrite the platform layer.

There is still alot of features that I would like to see here like file io, memory managment stuff etc. but the idea is that it will expand eventually while in use, list of current features:

* WASAPI sound initialization and basic sine wave example
* OpenGL rendering initialization and basic triangle example
* Console attach and console logging interface
* Hotreload / livereload (compile the app and changes apply automatically without closing it, yay)
* Input (Keyboard, mouse, xbox controller) interface
* Locked framerate / high precision timers 
* String library

## How to build 

Prerequisite: https://visualstudio.microsoft.com/pl/downloads/ (preferably VS Studio 2019 Community)(utility scripts wont work for VS newer than 2019)

1. Run the msvc-full-build.bat

## Troubleshooting

If upgrade_cmd64.bat has problems finding "vcvarsall.bat" then you need to manualy seek the said script. It should be located in the Visual Studio folder, try using the search folders functionality of windows, vcvarsall upgrades the cmd so that cmd can find microsoft c compiler. Run the "vcvarsall.bat" in your terminal x64 before calling the build script.

Easier method: find a "Developer Command prompt" or "Native Tools x64 Command prompt" in the start menu > Visual Studio, Run the said prompt and call the build script from that prompt

## Resources:

* https://hero.handmade.network/episode/code - Mostly inspired by Casey's videos (1-25 + OpenGL + lots of random episodes) Superb series on programming in general, I learned a ton.
* https://github.com/ryanfleury/app_template - helped me a ton with setting up WASAPI in C, "include trick" -> opengl_procedures.include.
* https://gist.github.com/mmozeiko/38c64bb65855d783645c#file-win32_handmade-cpp-L879 - Help with WASAPI 
* https://hero.handmade.network/forums/code-discussion/t/102-day_19_-_audio_latency - Help with WASAPI


