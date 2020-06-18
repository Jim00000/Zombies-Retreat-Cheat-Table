#include <QDebug>

#include "cheatbinder.h"
#include "ZRCheat/CheatingHelper.h"
#include "ZRCheat/ZombieRetreatCheat.h"

CheatBinder::CheatBinder(QObject *parent) : QObject(parent)
{

}

bool CheatBinder::findProcess()
{
    using namespace Cheating::Helper;
    DWORD pid = GetProcessIdByName(L"Zombie's Retreat - Beta 0.14.4.exe");

    if(pid == 0) {
        MessageBoxA(NULL, "Can not find process id of \"Zombie's Retreat - Beta 0.14.4.exe\"", "Error", MB_OK);
        return false;
    }

    this->hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if(this->hProc == NULL) {
        MessageBoxA(NULL, "Can not open process of \"Zombie's Retreat - Beta 0.14.4.exe\"", "Error", MB_OK);
        return false;
    }

    if (GetProcessBaseAddr(pid, pBase, baseSz) == false) {
        CloseHandle(this->hProc);
        MessageBoxA(NULL, "Can not get base address and base size of \"Zombie's Retreat - Beta 0.14.4.exe\" process", "Error", MB_OK);
        return false;
    }

    return true;
}

void CheatBinder::showProcessIsReady()
{
    MessageBoxA(NULL, "Get Ready. You can use cheat right now", "Information", MB_OK);
}

bool CheatBinder::enableAmmoHerbCheat()
{
    using namespace ZombieRetreat::Cheat::Patch;
    return PatchAmmoAndHerb99(this->hProc, this->pBase, this->baseSz);
}

bool CheatBinder::disableAmmoHerbCheat()
{
    using namespace ZombieRetreat::Cheat::Patch;
    return UnpatchAmmoAndHerb99(this->hProc, this->pBase, this->baseSz);
}

bool CheatBinder::enableHeldItem90Cheat()
{
    using namespace Cheating::Helper;
    using namespace ZombieRetreat::Cheat::Patch;

    if(this->pBaseOfNWDll == NULL) {
        if (GetModuleInfo(this->hProc, L"nw.dll", this->pBaseOfNWDll, this->NWDllImageSz) == FALSE) {
            return false;
        }
    }

    return PatchHeldItem90(this->hProc, reinterpret_cast<LPBYTE>(this->pBaseOfNWDll), this->NWDllImageSz);
}

bool CheatBinder::disableHeldItem90Cheat()
{
    using namespace ZombieRetreat::Cheat::Patch;
    return UnpatchHeldItem(this->hProc, reinterpret_cast<LPBYTE>(this->pBaseOfNWDll), this->NWDllImageSz);
}

bool CheatBinder::enableAllItem90Cheat()
{
    using namespace Cheating::Helper;
    using namespace ZombieRetreat::Cheat::Patch;

    if(this->pBaseOfNWDll == NULL) {
        if (GetModuleInfo(this->hProc, L"nw.dll", this->pBaseOfNWDll, this->NWDllImageSz) == FALSE) {
            return false;
        }
    }

    return PatchAllItem90(this->hProc, reinterpret_cast<LPBYTE>(this->pBaseOfNWDll), this->NWDllImageSz);
}

bool CheatBinder::disableAllItem90Cheat()
{
    using namespace ZombieRetreat::Cheat::Patch;

    return UnpatchAllItem90(this->hProc, reinterpret_cast<LPBYTE>(this->pBaseOfNWDll), this->NWDllImageSz);
}
