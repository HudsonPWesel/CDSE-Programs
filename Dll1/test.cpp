#include <Windows.h>
#include <iostream>

// VirtualAlloc prototype
LPVOID WINAPI HookedVirtualAlloc(LPVOID, SIZE_T, DWORD, DWORD);

// Variables

using VirtualAllocFunc = LPVOID(WINAPI *)(LPVOID, SIZE_T, DWORD, DWORD);
VirtualAllocFunc originalVirtualAlloc = VirtualAlloc;

BYTE originalBytes[12];
DWORD oldProtect;

// Function for restoring the original 12 first bytes for the VirtualAlloc function
VOID restoringOriginalVirtualAlloc()
{

    VirtualProtect(originalVirtualAlloc, sizeof(LPVOID), PAGE_EXECUTE_READWRITE, &oldProtect);

    memcpy(originalVirtualAlloc, originalBytes, sizeof(originalBytes));

    VirtualProtect(originalVirtualAlloc, sizeof(LPVOID), oldProtect, &oldProtect);
}

// Setting up the hook on VirtualAlloc so that the execution flow is redirected to HookedVirtualAlloc
VOID setHookOnVirtualAlloc()
{

    // Changing the protection of the original VirtualAlloc function to PAGE_EXECUTE_READWRITE (RWX) to be able to modify it
    VirtualProtect(originalVirtualAlloc, sizeof(LPVOID), PAGE_EXECUTE_READWRITE, &oldProtect);

    // Opcode for the incoditionnal jump to HookedVirtualAlloc whose address is stored in RAX
    uint8_t absJumpInstructions[] =
        {
            // mov rax, HookedVirtualAlloc address (48 B8 HookedVirtualAlloc)
            // jmp rax (FF E0)

            0x48, 0xB8,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Address placeholder - the address is 8 bytes long (64 bits)
            0xFF, 0xE0

        };

    uint64_t HookedVirtualAllocAddr = (uint64_t)(HookedVirtualAlloc); // Address of HookedVirtualAlloc

    // Replacing the HookedVirtualAlloc address placeholder in absJumpInstructions with the actual function address
    memcpy(&absJumpInstructions[2], &HookedVirtualAllocAddr, sizeof(HookedVirtualAllocAddr));

    // Replacing the first instruction of VirtualAlloc by an unconditional jump to HookedVirtualAlloc
    memcpy(originalVirtualAlloc, absJumpInstructions, sizeof(absJumpInstructions));

    // Restoring the original protection after the hook has been placed
    VirtualProtect(originalVirtualAlloc, sizeof(LPVOID), oldProtect, &oldProtect);
}

// Definition of the HookedVirtualAlloc function
LPVOID WINAPI HookedVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
    char debugMessage[512];
    snprintf(debugMessage, sizeof(debugMessage),
             "[HookedVirtualAlloc] lpAddress: 0x%p, dwSize: 0x%zx, flAllocationType: 0x%08X, flProtect: 0x%08X",
             lpAddress, dwSize, flAllocationType, flProtect);

    OutputDebugStringA(debugMessage);

    restoringOriginalVirtualAlloc(); // Restore the original VirtualAlloc before calling it

    LPVOID result = originalVirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);

    setHookOnVirtualAlloc(); // Reinstall the hook after the original call

    return result;
}

int main()
{
    memcpy(originalBytes, originalVirtualAlloc, sizeof(originalBytes));

    setHookOnVirtualAlloc();

    // Calling the hooked function
    LPVOID startAddress = VirtualAlloc(NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (startAddress != nullptr)
    {
        printf("Allocated memory start address: %p", startAddress);
    }

    return 0;
}

// #include <Windows.h>
// #include <iostream>
//
// int main() {
//     size_t memSize = 4096;
//
//     // Allocating 2 KB of virtual memory
//     LPVOID pMemory = VirtualAlloc(nullptr, memSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
//
//     if (pMemory != nullptr) {
//
//         // Data to copy
//         const char* dummyData = "Hack The Box";
//
//         // Copying data into the allocated memory
//         memcpy(pMemory, dummyData, strlen(dummyData) + 1);
//
//         // Displaying the contents of the allocated memory
//         std::cout << "Contents of allocated memory: " <<  (unsigned char*)(pMemory) << std::endl;
//
//         // Freeing the previously allocated memory
//         VirtualFree(pMemory, 0, MEM_RELEASE);
//     }
//
//     return 0;
// }