// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"


bool Hook(void* toHook, void* ourHook, int len)
{
	if (len < 5)
	{
		return false;
	}

	DWORD curProtection;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection);
	memset(toHook, 0x90, len);

	DWORD relativeAddress = ((DWORD)ourHook - (DWORD)toHook) - 5;

	*(BYTE*)toHook = 0xE9;
	*(DWORD*)((DWORD)toHook + 1) = relativeAddress;

	DWORD temp;
	VirtualProtect(toHook, len, curProtection, &temp);

	return true;
}


DWORD jmpBackAddy;

void __declspec(naked) ourFunct()
{
	__asm {
		//
		add ecx, ecx
		mov edx, [ebp-8]
		jmp [jmpBackAddy]
	}
}


DWORD WINAPI MainThread(LPVOID param)
{
	int hookLen = 6;
	DWORD hookAddress = 0x72768;
	jmpBackAddy = hookAddress + hookLen;
	Hook((void*)hookAddress, ourFunct, hookLen);
	while (true)
	{
		if (GetAsyncKeyState(VK_ESCAPE)) break;
		Sleep(50);
	}

	FreeLibraryAndExitThread((HMODULE)param, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
		break;
	}
	return TRUE;
}
