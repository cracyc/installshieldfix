#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "detours.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	STARTUPINFOA sia = {0};
	PROCESS_INFORMATION pi;
	MSG msg;
	char exename[MAX_PATH];
	char *file;
	PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE | PM_NOYIELD); // create message queue
	sia.cb = sizeof(STARTUPINFOA);
	sia.dwFlags = STARTF_USESHOWWINDOW;
	sia.wShowWindow = nShowCmd;
	GetModuleFileNameA(NULL, exename, MAX_PATH);
	file = strrchr(exename, '\\');
	if(!file) return -1;
	strcpy(file, "\\isfix32.dll");
	if(DetourCreateProcessWithDllExA(NULL, lpCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &sia, &pi, exename, NULL))
	{
		Sleep(200);
		WaitForInputIdle(pi.hProcess, 30000);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return 0;
}
