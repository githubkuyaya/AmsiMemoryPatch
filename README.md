# AmsiMemoryPatch
Contains a CPP program that bypasses AMSI

## Compile
As the code uses the Windows API, it is necessary to have the required headers installed.

### Windows
If you have `gcc` already installed, jump to step 3

1. Install [scoop](https://scoop.sh/)
2. Use `scoop` to install `gcc`: `scoop install gcc`
3. Compile: `g++ AmsiMemoryPatch.cpp -o AmsiMemoryPatch.exe`

### Linux
If you have `mingw-w64-gcc` already installed, jump to step 2

0. Install Arch
1. Install `mingw-w64`: `pacman -S mingw-w64-gcc`
2. Compile: `x86_64-w64-mingw32-g++ -static AmsiMemoryPatch.cpp -o AmsiMemoryPatch.exe`

### MacOS
0. Install Asahi Linux
1. Follow the [linux](#linux) installation instructions
