# AmsiMemoryPatch
Contains a CPP program that bypasses AMSI

## Compile
As the code uses the Windows API, it is necessary to have the required headers installed.

### Windows
If you have `gcc` already installed, jump to step 3

1. Install [scoop](https://scoop.sh/)
2. Use `scoop` to install `gcc`: `scoop install gcc`
3. Compile: `g++ AmsiMemoryPatch.cpp -o AmsiMemoryPatch.exe -fpermissive`

`cl .\AmsiMemoryPatch.cpp /EHsc /link /out:AmsiMemoryPatch.exe`

`g++ .\AmsiMemoryPatch.cpp -fpermissive -o AmsiMemoryPatch.exe`

Or paste it into visual studio and hit `build`

### Linux
If you have `mingw-w64-gcc` already installed, jump to step 2

0. Install Arch
1. Install `mingw-w64`: `pacman -S mingw-w64-gcc`
2. Compile: `x86_64-w64-mingw32-g++ -static AmsiMemoryPatch.cpp -o AmsiMemoryPatch.exe -fpermissive`

### MacOS
0. Install Asahi Linux
1. Follow the [linux](#linux) installation instructions

## Weirdness / Issues
* You are able to overwrite the memory, even though the memory protection status is set to 0x20 = RX (no write). The code theoretically also works without the `VirtualProtectEx` function call. Check the docs [here](https://learn.microsoft.com/en-us/windows/win32/memory/memory-protection-constants)
* You are able to query the permissions, even though OpenProcess is called without the PROCESS_QUERY_INFORMATION parameter. The [docs](https://learn.microsoft.com/en-us/windows/win32/procthread/process-security-and-access-rights) state that this permission is required to be able to query information about a process.
* In a normal PowerShell window, the code works as expected with all compilers. However, when opening a Developer Powershell window and trying to patch said process (`--parent`), only the version compiled with `cl` seems to be able to patch AMSI. The other two return `0x1` for `VirtualQueryEx` and `0x1E7` for `VirtualProtectEx`. Ignoring `VirtualProtectEx` and directly trying `WriteProcessMemory` with `0x3E6`.
* The version compiled with `cl` returns different addresses for `LoadLibraryA` and `GetProcAddress`. This is probably related to the issue listed above. 
* The version compiled with `g++` prints the addresses in lowercase.
