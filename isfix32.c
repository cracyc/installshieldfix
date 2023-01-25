#define WIN32_LEAN_AND_MEAN
#define __USE_MINGW_ANSI_STDIO 0
#include <windows.h>
#include "detours.h"

typedef LRESULT (WINAPI *SendMessageA_t)(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
SendMessageA_t SendMessageAOrig;

LRESULT WINAPI SendMessageAHook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if(((DWORD)hwnd & 0xffff) == 0xffff) // user treats 0xffff as HWND_BROADCAST
		return SendMessageTimeoutA(hwnd, msg, wparam, lparam, SMTO_ABORTIFHUNG, 100, NULL);
	return SendMessageAOrig(hwnd, msg, wparam, lparam);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hinstDLL);
			HMODULE user32 = GetModuleHandleA("user32.dll");
			SendMessageAOrig = (SendMessageA_t)GetProcAddress(user32, "SendMessageA");
			DetourRestoreAfterWith();

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID)SendMessageAOrig, SendMessageAHook);
			LONG error = DetourTransactionCommit();

			break;
		}
		case DLL_PROCESS_DETACH:
		{
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourDetach(&(PVOID)SendMessageAOrig, SendMessageAHook);
			LONG error = DetourTransactionCommit();
		}
	}
	return TRUE;
}

__declspec(dllexport) void dummy()
{
}
