This repository contains three components - Gen, Compiler Proxy and Example.

# Gen
A C++20, header only library for arbitrary compile time execution and code generation.

A detailed explanation of the library, its implementation and how to use it, is available at this Core C++ talk:
https://www.youtube.com/watch?v=3aI3Mh2SE_Y

Old blog post about the previous implementation of the library (outdated):
https://a10nw01f.github.io/generative_cpp/

# Compiler Proxy Dependencies
Subprocess - used for process creation: https://github.com/sheredom/subprocess.h

Compiler proxy currently only supports windows as development platform. 
Most of the transition to cross platform has already been completed. 

# Example Dependencies
In order to run the HelloCpp2 example it needs the "cpp2util.h" file from the cppfront repository: https://github.com/hsutter/cppfront

# Compiling the Example
1. Download or clone the github repository.
2. Install xmake if you don't have it installed already: https://xmake.io/#/
3. Compile the compiler proxy: `xmake -r CompilerProxy`
4. Create a folder and add it to the beginning of your path environment variable for example:
`mkdir C:\priority_path`
`setx path "C:\priority_path;%path%"`
You can change it from `C:\priority_path` to any path you want.
5. Copy CompilerProxy.exe to the folder you created and rename it after the name of your compiler (for example cl.exe/gcc.exe/clang.exe): `copy ".\build\windows\x64\release\CompilerProxy.exe" c:\priority_path\cl.exe`
6. Create a config.txt in the folder that you created which maps to actual compiler executable.
Example of a config.txt file content:
`gcc.exe`
`C:\msys64\mingw64\bin\gcc.exe` 
`clang.exe`
`C:\msys64\mingw64\bin\clang.exe` 
`cl.exe`
`C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.34.31933\bin\Hostx64\x64\cl.exe`

7. Compile the example: `xmake -r Example`

You can uncomment the `add_defines("CPP2EXAMPLE")` inside xmake.lua to compile the cpp2 example but you need to have cppfront.exe since it will try to create a process with the following command: `cppfront <some args>`.
