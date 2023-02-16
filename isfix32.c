#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "detours.h"

typedef LRESULT (WINAPI *SendMessageA_t)(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
typedef LRESULT (WINAPI *CreateProcessA_t)(LPCSTR name, LPSTR cmdline, LPSECURITY_ATTRIBUTES procattr, LPSECURITY_ATTRIBUTES thrdattr, BOOL inherit, DWORD cflags, LPVOID env, LPCSTR cdir, LPSTARTUPINFOA sinfo, LPPROCESS_INFORMATION pinfo);

SendMessageA_t SendMessageAOrig;
CreateProcessA_t CreateProcessAOrig;
HMODULE hmod;

LRESULT WINAPI SendMessageAHook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if(((DWORD)hwnd & 0xffff) == 0xffff) // user treats 0xffff as HWND_BROADCAST
		return SendMessageTimeoutA(hwnd, msg, wparam, lparam, SMTO_ABORTIFHUNG, 100, NULL);
	return SendMessageAOrig(hwnd, msg, wparam, lparam);
}

LRESULT WINAPI CreateProcessAHook(LPCSTR name, LPSTR cmdline, LPSECURITY_ATTRIBUTES procattr, LPSECURITY_ATTRIBUTES thrdattr, BOOL inherit, DWORD cflags, LPVOID env, LPCSTR cdir, LPSTARTUPINFOA sinfo, LPPROCESS_INFORMATION pinfo)
{
	HMODULE module;
	char dllname[MAX_PATH];
	GetModuleFileNameA(hmod, dllname, MAX_PATH);
	return DetourCreateProcessWithDllExA(name, cmdline, procattr, thrdattr, inherit, cflags, env, cdir, sinfo, pinfo, dllname, CreateProcessAOrig);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
		{
			hmod = (HMODULE)hinstDLL;
			DisableThreadLibraryCalls(hinstDLL);
			HMODULE user32 = GetModuleHandleA("user32.dll");
			HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
			SendMessageAOrig = (SendMessageA_t)GetProcAddress(user32, "SendMessageA");
			CreateProcessAOrig = (CreateProcessA_t)GetProcAddress(kernel32, "CreateProcessA");
			DetourRestoreAfterWith();

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID)SendMessageAOrig, SendMessageAHook);
			DetourAttach(&(PVOID)CreateProcessAOrig, CreateProcessAHook);
			LONG error = DetourTransactionCommit();

			break;
		}
		case DLL_PROCESS_DETACH:
		{
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourDetach(&(PVOID)SendMessageAOrig, SendMessageAHook);
			DetourDetach(&(PVOID)CreateProcessAOrig, CreateProcessAHook);
			LONG error = DetourTransactionCommit();
		}
	}
	return TRUE;
}

__declspec(dllexport) void dummy()
{
}
