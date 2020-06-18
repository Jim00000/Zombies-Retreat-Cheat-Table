#include "pch.h"

#include <tlhelp32.h>
#include <Psapi.h>
#include "spdlog_wrapper.h"

VOID Cheating::Helper::CheckLastError()
{
	LPSTR lpMsgBuf;

	SIZE_T size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0,
		NULL
	);

	std::string message(lpMsgBuf, size);
	LocalFree(lpMsgBuf);
	spdlog::error(message);
}

BOOL Cheating::Helper::GetProcessBaseAddr(DWORD pid, LPBYTE& baseAddr, DWORD& baseSize) {

	HANDLE hSnapshot;
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, pid);
	
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		CheckLastError();
		return FALSE;
	}

	// Designated initializers
	MODULEENTRY32W modEntry = {
		.dwSize = sizeof(MODULEENTRY32W)
	};

	if (Module32First(hSnapshot, &modEntry)) {
		baseAddr = modEntry.modBaseAddr;
		baseSize = modEntry.modBaseSize;
	}

	CloseHandle(hSnapshot);

	return TRUE;
}

DWORD Cheating::Helper::GetProcessIdByName(LPCWSTR procName)
{
	DWORD processId = 0;
	SIZE_T workingSetSz = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE) {
		CheckLastError();
		return FALSE;
	}

	PROCESSENTRY32W procEntry = {
		.dwSize = sizeof(PROCESSENTRY32W)
	};

	if (Process32FirstW(hSnapshot, &procEntry)) {
		while (Process32Next(hSnapshot, &procEntry) && GetLastError() != ERROR_NO_MORE_FILES) {
			if (wcscmp(procName, procEntry.szExeFile) == 0) {
				const HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procEntry.th32ProcessID);
				PROCESS_MEMORY_COUNTERS memCounters;
				if (GetProcessMemoryInfo(hProcess, &memCounters, sizeof(memCounters)) == 0)
					CheckLastError();
				if (workingSetSz < memCounters.WorkingSetSize) {
					workingSetSz = memCounters.WorkingSetSize;
					processId = procEntry.th32ProcessID;
				}
				CloseHandle(hProcess);
			}
		}
	}

	CloseHandle(hSnapshot);
	return processId;
}

DWORD64 Cheating::Helper::AOBScan(HANDLE hProcess, LPBYTE baseAddr, DWORD baseSize, LPBYTE victim, SIZE_T victimSz)
{
	using namespace Cheating::Helper;

	DWORD64 victimAddr = NULL;
	LPBYTE buffer = new BYTE[baseSize];
	ZeroMemory(buffer, baseSize);

	if (ReadProcessMemory(hProcess, baseAddr, buffer, baseSize, NULL) == 0) {
		CheckLastError();
		delete[] buffer;
		exit(EXIT_FAILURE);
	}

	for (SIZE_T i = 0; i < baseSize - victimSz; i++) {
		if (memcmp(&buffer[i], victim, victimSz) == 0) {
			victimAddr = reinterpret_cast<DWORD64>(baseAddr + i);
			break;
		}
	}

	delete[] buffer;
	return victimAddr;
}

DWORD64 Cheating::Helper::AOBScanEx(HANDLE hProcess, LPBYTE baseAddr, LPBYTE victim, SIZE_T victimSz)
{
	DWORD64 victimAddr = 0;
	DWORD64 startAddr = (DWORD64)baseAddr;
	while (startAddr < 0x7FFFFFFFFFFF && victimAddr == 0) {
		MEMORY_BASIC_INFORMATION pageInfo = { 0 };

		if (VirtualQueryEx(hProcess, (LPVOID)startAddr, &pageInfo, sizeof(MEMORY_BASIC_INFORMATION)) == 0) {
			CheckLastError();
			exit(EXIT_FAILURE);
		}

		if (pageInfo.AllocationProtect == PAGE_NOACCESS &&
			pageInfo.State == MEM_COMMIT &&
			pageInfo.Type == MEM_PRIVATE &&
			pageInfo.Protect == PAGE_EXECUTE_READWRITE) 
		{
			victimAddr = AOBScan(hProcess, (LPBYTE)pageInfo.BaseAddress, pageInfo.RegionSize, victim, victimSz);
		}

		// For next iteration
		startAddr = (DWORD64)pageInfo.BaseAddress + pageInfo.RegionSize;
	}

	return victimAddr;
}

BOOL Cheating::Helper::GetModuleInfo(HANDLE hProcess, LPCWSTR moduleName, LPVOID& baseOfDll, DWORD& sizeOfImage)
{
	HMODULE* hModules = nullptr;
	DWORD moduleBytes = 0;
	DWORD moduleSz = 0;

	if (EnumProcessModulesEx(hProcess, hModules, 0, &moduleBytes, LIST_MODULES_ALL) == 0)
		CheckLastError();

	moduleSz = moduleBytes / sizeof(HMODULE);
	hModules = new HMODULE[moduleSz];

	if (EnumProcessModulesEx(hProcess, hModules, sizeof(HMODULE) * moduleSz, &moduleBytes, LIST_MODULES_ALL) == 0)
		CheckLastError();
	
	for (SIZE_T i = 0; i < moduleSz; i++) {
		MODULEINFO info;
		WCHAR baseName[256];
		ZeroMemory(baseName, 256);
		if (GetModuleInformation(hProcess, hModules[i], &info, sizeof(MODULEINFO)) == 0)
			CheckLastError();
		if (GetModuleBaseNameW(hProcess, hModules[i], baseName, 256) == 0)
			CheckLastError();

		// spdlog::debug(L"Base Address : {:x}, size : {:d}, module name: {}", (DWORD64)info.lpBaseOfDll, info.SizeOfImage, baseName);

		if (wcscmp(baseName, moduleName) == 0) {
			baseOfDll = info.lpBaseOfDll;
			sizeOfImage = info.SizeOfImage;
			spdlog::info(L"Base Address : {:x}, size : {:d}, module name: {}", (DWORD64)info.lpBaseOfDll, info.SizeOfImage, baseName);
		}

	}

	delete[] hModules;
	return TRUE;
}

LPVOID Cheating::Helper::NewMemoryBlock(HANDLE hProcess, SIZE_T size)
{
	LPVOID newPage = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (newPage == NULL) {
		CheckLastError();
	}

	return newPage;
}

BOOL Cheating::Helper::FreeMemoryBlock(HANDLE hProcess, LPVOID lpAddress)
{
	if (VirtualFreeEx(hProcess, lpAddress, 0, MEM_RELEASE) == 0) {
		CheckLastError();
		return FALSE;
	}

	return TRUE;
}