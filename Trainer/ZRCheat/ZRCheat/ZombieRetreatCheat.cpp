#include "pch.h"

namespace {
    
    /* Ammo & herb */
    BOOL gIsAmmoHerb99Patched = FALSE;
    LPVOID gAmmoHerb99NewMemBlock = NULL;
    LPVOID gAmmoHerb99PatchCode = NULL;
    BYTE gVictimOfAmmoHerb99[] = {
        0x03,                                       // ...
        0xD1, 0xFF,                                 // sar edi, 1
        0x3B, 0xDF,                                 // cmp ebx, edi
        0x73, 0x20,                                 // jae ...
        0x8D, 0x0C, 0x9D, 0x07, 0x00, 0x00, 0x00,   // lea ecx, [ebx*4+00000007]
        0x8B, 0x5C, 0x24, 0x0C,                     // mov ebx, [esp+0C]
        0x89, 0x1C, 0x08,                           // mov [eax+ecx], ebx
        0xBE, 0x00, 0x00, 0xF8, 0xFF                // mov esi, FFF80000
    };
    constexpr SIZE_T gVictimOfAmmoHerb99Sz = sizeof(gVictimOfAmmoHerb99) / sizeof(gVictimOfAmmoHerb99[0]);

    /* Held item x90 */
    BOOL gIsHeldItem90Patched = FALSE;
    LPVOID gHeldItem90NewMemBlock = NULL;
    LPVOID gHeldItem90PatchCode = NULL;
    BYTE gVictim[] = {
        0x8B, 0x44, 0xB8, 0x07, // mov eax, [eax + edi * 4 + 07]
        0x8B, 0x4D, 0xDC        // mov ecx, [ebp - 24]
    };
    constexpr SIZE_T gVictimSz = sizeof(gVictim) / sizeof(gVictim[0]);

    /* All item x90 */
    BOOL gIsAllItem90Patched = FALSE;
    LPVOID gAllItem90NewMemBlock = NULL;
    LPVOID gAllItem90PatchCode = NULL;

}

BOOL ZombieRetreat::Cheat::Patch::PatchHeldItem90(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize)
{
    using namespace Cheating::Helper;

    if (gIsHeldItem90Patched == TRUE) {
        spdlog::error("\"Held item x90\" patch is applied. You can not patch the code again.");
        return FALSE;
    }

    DWORD64 victimAddr = AOBScan(hProcess, baseAddr, baseSize, gVictim, gVictimSz);
    spdlog::debug("AOBScan : {:016x}", victimAddr);

    if (victimAddr == NULL) {
        spdlog::error("Cannot find the victim code by AOBScan. Abort {} function", __FUNCTION__);
        return FALSE;
    }

    LPVOID rNewMemBlock = NewMemoryBlock(hProcess, 4096);
    spdlog::debug("Remote new memory block : {:016x}", reinterpret_cast<DWORD64>(rNewMemBlock));

    
    { // Patch instruction to selected point
        BYTE shellcode[] = {
            0xE9, 0x00, 0x00, 0x00, 0x00,   // jmp ...
            0x66, 0x90                      // xchg ax, ax
        };
        constexpr SIZE_T shellcodeSz = sizeof(shellcode) / sizeof(shellcode[0]);
        static_assert(shellcodeSz == gVictimSz, "shellcodeSz must be equal to gVictimSz");

        // Point to the constant field of jmp instruction
        DWORD32* pImm = (DWORD32*)&shellcode[1];
        // Write displacement to the constant field of jmp instruction
        *pImm = (DWORD32)((DWORD64)rNewMemBlock - (DWORD64)(victimAddr + 5));

        if (WriteProcessMemory(hProcess, (LPVOID)victimAddr, shellcode, shellcodeSz, NULL) == 0) {
            CheckLastError();
            FreeMemoryBlock(hProcess, rNewMemBlock);
            return FALSE;
        }
    }

    { // Write data to memory block
        BYTE shellcode[] = {
            0x81, 0x7C, 0xB8, 0x07, 0xC8, 0x00, 0x00, 0x00, // cmp[eax + edi * 4 + 07], C8
            0x7D, 0x08,                                     // jnl original_code                               
            0xC7, 0x44, 0xB8, 0x07, 0xB4, 0x00, 0x00, 0x00, // mov[eax + edi * 4 + 07], B4
            // original_code:
            0x8B, 0x44, 0xB8, 0x07,                         // mov eax, [eax + edi * 4 + 07]
            0x8B, 0x4D, 0xDC,                               // mov ecx, [ebp - 24]
            0xE9, 0x00, 0x00, 0x00, 0x00                    // jmp ...
        };
        constexpr SIZE_T shellcodeSz = sizeof(shellcode) / sizeof(shellcode[0]);

        DWORD32* pImm = (DWORD32*)&shellcode[26];
        // Write return address (back to vicitm code)
        // Note that the displacement is written in actually
        *pImm = (DWORD32)((DWORD64)(victimAddr + gVictimSz) - ((DWORD64)rNewMemBlock + shellcodeSz));


        if (WriteProcessMemory(hProcess, rNewMemBlock, shellcode, shellcodeSz, NULL) == 0) {
            CheckLastError();
            FreeMemoryBlock(hProcess, rNewMemBlock);
            return FALSE;
        }
    }

    gIsHeldItem90Patched = TRUE;
    gHeldItem90NewMemBlock = rNewMemBlock;
    gHeldItem90PatchCode = reinterpret_cast<decltype(gHeldItem90PatchCode)>(victimAddr);

    return TRUE;
}

BOOL ZombieRetreat::Cheat::Patch::UnpatchHeldItem(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize)
{
    using namespace Cheating::Helper;

    if (gIsHeldItem90Patched == FALSE) {
        spdlog::error("\"Held item x90\" patch is not applied");
        return FALSE;
    }

    if (gHeldItem90PatchCode == NULL) {
        spdlog::error("We do not know where the \"Held item x90\" patch is applied to");
        return FALSE;
    }

    if (gHeldItem90NewMemBlock == NULL) {
        spdlog::error("We do not know where is the memory block of \"Held item x90\" cheat");
        return FALSE;
    }

    if (WriteProcessMemory(hProcess, (LPVOID)gHeldItem90PatchCode, gVictim, gVictimSz, NULL) == 0) {
        CheckLastError();
        return FALSE;
    }

    gIsHeldItem90Patched = FALSE;
    gHeldItem90PatchCode = NULL;

    if (FreeMemoryBlock(hProcess, gHeldItem90NewMemBlock) == FALSE) {
        CheckLastError();
        return FALSE;
    }

    gHeldItem90NewMemBlock = NULL;

    return TRUE;
}

BOOL ZombieRetreat::Cheat::Patch::PatchAllItem90(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize)
{
    using namespace Cheating::Helper;

    if (gIsAllItem90Patched == TRUE) {
        spdlog::error("\"All items x90\" patch is applied. You can not patch the code again.");
        return FALSE;
    }

    if (gIsHeldItem90Patched == TRUE) {
        spdlog::error("\"Held item x90\" patch is applied. You can not patch the code again.");
        return FALSE;
    }

    DWORD64 victimAddr = AOBScan(hProcess, baseAddr, baseSize, gVictim, gVictimSz);
    spdlog::debug("AOBScan : {:016x}", victimAddr);

    if (victimAddr == NULL) {
        spdlog::error("Cannot find the victim code by AOBScan. Abort {} function", __FUNCTION__);
        return FALSE;
    }

    LPVOID rNewMemBlock = NewMemoryBlock(hProcess, 4096);
    spdlog::debug("Remote new memory block : {:016x}", reinterpret_cast<DWORD64>(rNewMemBlock));

    { // Patch instruction to selected point
        BYTE shellcode[] = {
            0xE9, 0x00, 0x00, 0x00, 0x00,   // jmp ...
            0x66, 0x90                      // xchg ax, ax
        };
        constexpr SIZE_T shellcodeSz = sizeof(shellcode) / sizeof(shellcode[0]);
        static_assert(shellcodeSz == gVictimSz, "shellcodeSz must be equal to gVictimSz");

        // Point to the constant field of jmp instruction
        DWORD32* pImm = (DWORD32*)&shellcode[1];
        // Write displacement to the constant field of jmp instruction
        *pImm = (DWORD32)((DWORD64)rNewMemBlock - (DWORD64)(victimAddr + 5));

        if (WriteProcessMemory(hProcess, (LPVOID)victimAddr, shellcode, shellcodeSz, NULL) == 0) {
            CheckLastError();
            FreeMemoryBlock(hProcess, rNewMemBlock);
            return FALSE;
        }
    }

    { // Write data to memory block
        BYTE shellcode[] = {
            0xC7, 0x44, 0xB8, 0x07, 0xB4, 0x00, 0x00, 0x00, // mov [eax+edi*4+07], B4
            // original_code:
            0x8B, 0x44, 0xB8, 0x07,                         // mov eax, [eax + edi * 4 + 07]
            0x8B, 0x4D, 0xDC,                               // mov ecx, [ebp - 24]
            0xE9, 0x00, 0x00, 0x00, 0x00                    // jmp ...
        };
        constexpr SIZE_T shellcodeSz = sizeof(shellcode) / sizeof(shellcode[0]);

        DWORD32* pImm = (DWORD32*)&shellcode[16];
        // Write return address (back to vicitm code)
        // Note that the displacement is written in actually
        *pImm = (DWORD32)((DWORD64)(victimAddr + gVictimSz) - ((DWORD64)rNewMemBlock + shellcodeSz));


        if (WriteProcessMemory(hProcess, rNewMemBlock, shellcode, shellcodeSz, NULL) == 0) {
            CheckLastError();
            FreeMemoryBlock(hProcess, rNewMemBlock);
            return FALSE;
        }
    }

    gIsAllItem90Patched = TRUE;
    gAllItem90NewMemBlock = rNewMemBlock;
    gAllItem90PatchCode = reinterpret_cast<decltype(gAllItem90PatchCode)>(victimAddr);

    return TRUE;
}

BOOL ZombieRetreat::Cheat::Patch::UnpatchAllItem90(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize)
{
    using namespace Cheating::Helper;

    if (gIsAllItem90Patched == FALSE) {
        spdlog::error("\"All items x90\" patch is not applied");
        return FALSE;
    }

    if (gAllItem90PatchCode == NULL) {
        spdlog::error("We do not know where the \"All items x90\" patch is applied to");
        return FALSE;
    }

    if (gAllItem90NewMemBlock == NULL) {
        spdlog::error("We do not know where is the memory block of \"All items x90\" cheat");
        return FALSE;
    }

    if (WriteProcessMemory(hProcess, (LPVOID)gAllItem90PatchCode, gVictim, gVictimSz, NULL) == 0) {
        CheckLastError();
        return FALSE;
    }

    gIsAllItem90Patched = FALSE;
    gAllItem90PatchCode = NULL;

    if (FreeMemoryBlock(hProcess, gAllItem90NewMemBlock) == FALSE) {
        CheckLastError();
        return FALSE;
    }

    gAllItem90NewMemBlock = NULL;

    return TRUE;
}


BOOL ZombieRetreat::Cheat::Patch::PatchAmmoAndHerb99(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize)
{
    using namespace Cheating::Helper;

    if (gIsAmmoHerb99Patched == TRUE) {
        spdlog::error("\"All kinds of ammo and herb x99\" patch is applied. You can not patch the code again.");
        return FALSE;
    }

    DWORD64 victimAddr = AOBScanEx(hProcess, baseAddr, gVictimOfAmmoHerb99, gVictimOfAmmoHerb99Sz);
    spdlog::debug("AOBScanEx : {:016x}", victimAddr);

    if (victimAddr == NULL) {
        spdlog::error("Cannot find the victim code by AOBScanEx. Abort {} function", __FUNCTION__);
        return FALSE;
    }

    LPVOID rNewMemBlock = NewMemoryBlock(hProcess, 4096);
    spdlog::debug("Remote new memory block : {:016x}", reinterpret_cast<DWORD64>(rNewMemBlock));

    { // Patch instruction to selected point
        BYTE shellcode[] = {
            0x03,                                       // ...
            0xD1, 0xFF,                                 // sar edi, 1
            0x3B, 0xDF,                                 // cmp ebx, edi
            0x73, 0x20,                                 // jae ...
            0x8D, 0x0C, 0x9D, 0x07, 0x00, 0x00, 0x00,   // lea ecx, [ebx*4+00000007]
            0x8B, 0x5C, 0x24, 0x0C,                     // mov ebx, [esp+0C]
            // Start to patch here
            0xE9, 0x00, 0x00, 0x00, 0x00,               // jmp ...
            0x0F, 0x1F, 0x00                            // nop DWORD PTR [rax]
        };
        constexpr SIZE_T shellcodeSz = sizeof(shellcode) / sizeof(shellcode[0]);
        static_assert(shellcodeSz == gVictimOfAmmoHerb99Sz, "shellcodeSz must be equal to gVictimOfAmmoHerb99Sz");

        // Point to the constant field of jmp instruction
        DWORD32* pImm = (DWORD32*)&shellcode[19];
        // Write displacement to the constant field of jmp instruction
        *pImm = (DWORD32)((DWORD64)rNewMemBlock - (DWORD64)(victimAddr + 23));

        if (WriteProcessMemory(hProcess, (LPVOID)victimAddr, shellcode, shellcodeSz, NULL) == 0) {
            CheckLastError();
            FreeMemoryBlock(hProcess, rNewMemBlock);
            return FALSE;
        }
    }

    { // Write data to memory block
        BYTE shellcode[] = {
            0x83, 0xF9, 0x23,                           // cmp ecx,23
            0x74, 0x1F,                                 // je change
            0x83, 0xF9, 0x7F,                           // cmp ecx,7F
            0x74, 0x1A,                                 // je change
            0x81, 0xF9, 0xBF, 0x00, 0x00, 0x00,         // cmp ecx,000000BF
            0x74, 0x12,                                 // je change
            0x83, 0xF9, 0x1B,                           // cmp ecx,1B
            0x74, 0x0D,                                 // je change
            // original_code:                           // original_code:
            0x89, 0x1C, 0x08,                           // mov [eax+ecx],ebx
            // exit:                                    // exit:
            0xBE, 0x00, 0x00, 0xF8, 0xFF,               // mov esi, FFF80000
            0xE9, 0x00, 0x00, 0x00, 0x00,               // jmp ...
            // change:                                  // change:
            0xC7, 0x40, 0x23, 0xC6, 0x00, 0x00, 0x00,   // mov[eax + 23],000000C6
            0xC7, 0x40, 0x1B, 0xC6, 0x00, 0x00, 0x00,   // mov[eax + 1B],000000C6
            0xC7, 0x40, 0x7F, 0xC6, 0x00, 0x00, 0x00,   // mov[eax + 7F],000000C6
            0xC7, 0x80, 0xBF, 0x00, 0x00, 0x00,         // mov [eax+000000BF],000000C6
            0xC6, 0x00, 0x00, 0x00,                     //
            0xEB, 0xD5                                  // jmp exit

        };
        constexpr SIZE_T shellcodeSz = sizeof(shellcode) / sizeof(shellcode[0]);

        DWORD32* pImm = (DWORD32*)&shellcode[32];
        // Go back to vicitm code (Write displacement)
        *pImm = (DWORD32)((DWORD64)(victimAddr + gVictimOfAmmoHerb99Sz) - ((DWORD64)rNewMemBlock + 36));


        if (WriteProcessMemory(hProcess, rNewMemBlock, shellcode, shellcodeSz, NULL) == 0) {
            CheckLastError();
            FreeMemoryBlock(hProcess, rNewMemBlock);
            return FALSE;
        }
    }

    gIsAmmoHerb99Patched = TRUE;
    gAmmoHerb99NewMemBlock = rNewMemBlock;
    gAmmoHerb99PatchCode = reinterpret_cast<decltype(gAmmoHerb99PatchCode)>(victimAddr);

    return TRUE;
}

BOOL ZombieRetreat::Cheat::Patch::UnpatchAmmoAndHerb99(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize)
{
    using namespace Cheating::Helper;

    if (gIsAmmoHerb99Patched == FALSE) {
        spdlog::error("\"All kinds of ammo and herb x99\" patch is not applied");
        return FALSE;
    }

    if (gAmmoHerb99PatchCode == NULL) {
        spdlog::error("We do not know where the \"All kinds of ammo and herb x99\" patch is applied to");
        return FALSE;
    }

    if (gAmmoHerb99NewMemBlock == NULL) {
        spdlog::error("We do not know where is the memory block of \"All kinds of ammo and herb x99\" cheat");
        return FALSE;
    }

    if (WriteProcessMemory(hProcess, (LPVOID)gAmmoHerb99PatchCode, gVictimOfAmmoHerb99, gVictimOfAmmoHerb99Sz, NULL) == 0) {
        CheckLastError();
        return FALSE;
    }

    gIsAmmoHerb99Patched = FALSE;
    gAmmoHerb99PatchCode = NULL;

    if (FreeMemoryBlock(hProcess, gAmmoHerb99NewMemBlock) == FALSE) {
        CheckLastError();
        return FALSE;
    }

    gAmmoHerb99NewMemBlock = NULL;

    return TRUE;
}
