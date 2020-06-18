#pragma once

#include <windows.h>

namespace ZombieRetreat::Cheat::Patch {

    __declspec(dllexport) BOOL PatchHeldItem90(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize);

    __declspec(dllexport) BOOL UnpatchHeldItem(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize);

    __declspec(dllexport) BOOL PatchAllItem90(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize);

    __declspec(dllexport) BOOL UnpatchAllItem90(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize);

    __declspec(dllexport) BOOL PatchAmmoAndHerb99(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize);

    __declspec(dllexport) BOOL UnpatchAmmoAndHerb99(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize);

}