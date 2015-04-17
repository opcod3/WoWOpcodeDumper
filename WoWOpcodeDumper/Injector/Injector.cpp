#include <windows.h>
#include <TlHelp32.h>
#include <shlwapi.h>
#include <conio.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#define CREATE_THREAD_ACCESS (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ)

BOOL Inject(DWORD pID, const char * DLL_NAME);
DWORD GetTargetThreadIDFromProcName(const char * ProcName);

int main(int argc, char * argv[])
{
	int processId = GetTargetThreadIDFromProcName("WowT_Patched.exe");
	char buffer[MAX_PATH] = { 0 };

	GetFullPathNameA("./OpcodeDumperDll.dll", MAX_PATH, buffer, NULL);

	if (!Inject(processId, buffer))
		printf("Dll could not attach.");
	else
		printf("Dll was properly loaded.");
	_getch();
	return 0;
}

BOOL Inject(DWORD pID, const char * DLL_NAME)
{
	HANDLE Proc;
	LPVOID remoteString, loadLibraryA;

	if (!pID)
		return false;

	Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

	if (!Proc)
		return false;

	loadLibraryA = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	remoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(DLL_NAME), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(Proc, (LPVOID)remoteString, DLL_NAME, strlen(DLL_NAME), NULL);
	HANDLE remoteThread = CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)loadLibraryA, (LPVOID)remoteString, NULL, NULL);

	if (remoteThread)
		WaitForSingleObject(remoteThread, INFINITE);

	VirtualFreeEx(Proc, remoteString, strlen(DLL_NAME), MEM_RELEASE);
	CloseHandle(Proc);
	return true;
}

DWORD GetTargetThreadIDFromProcName(const char * ProcName)
{
	tagPROCESSENTRY32 pe;
	HANDLE snapShot;
	BOOL retval, ProcFound = false;

	snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (snapShot == INVALID_HANDLE_VALUE)
		return false;

	pe.dwSize = sizeof(tagPROCESSENTRY32);

	retval = Process32First(snapShot, &pe);

	while (retval)
	{
		if (_stricmp(pe.szExeFile, ProcName) == 0)
		{
			return pe.th32ProcessID;
		}

		retval = Process32Next(snapShot, &pe);
	}
	return 0;
}