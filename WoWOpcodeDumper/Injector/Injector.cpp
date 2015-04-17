#include <windows.h>
#include <TlHelp32.h>
#include <shlwapi.h>
#include <conio.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN

// Possilbe names of the process to attach to
std::string processNames[] = { /*Used for setting a name with an argument*/"", "Wow.exe", "Wow_Patched.exe", "WowT.exe", "WoWT_Patched.exe" };
// Name of DLL to inject
char* dllToInjectName = "OpcodeDumper.dll";

// Injects DLL into a process
BOOL Inject(DWORD pID, const char * DLL_PATH);
// Gets pID of process found by name
DWORD GetTargetThreadID();
// Prompts user for input and pauses console
void PauseSystem();

int main(int argc, char * argv[])
{
    // Set pretty title
    SetConsoleTitle("WoWOpcodeDumper Injector");

    if (argc > 2)
    {
        printf("ERROR: Too many arguments.\n");
        printf("Usage is: \"Injector.exe [wow_process_name]\"\n");
        PauseSystem();
        return 1;
    }
    // Set custom process name
    else if (argc == 2)
        processNames[0] = std::string(argv[1]);

    // Get process in which to inject
	int processId = GetTargetThreadID();

    //Exit if no process found
    if (processId == 0)
    {
        printf("ERROR: Wow process not found!\n");
        PauseSystem();
        return 1;
    }

    // Get path of DLL to inject
	char buffer[MAX_PATH] = { 0 };
	GetFullPathNameA(dllToInjectName, MAX_PATH, buffer, NULL);


    // Try to inject
    if (!Inject(processId, buffer))
    {
        printf("ERROR: Dll failed to inject.\n");
        PauseSystem();
    }
	else
		printf("Dll has been injected!\n");

    printf("Exiting... ");
    Sleep(3000);
	return 0;
}

BOOL Inject(DWORD pID, const char * DLL_PATH)
{
	HANDLE Proc;
	LPVOID remoteString, loadLibraryA;

	Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

    if (!Proc)
    {
        printf("ERROR: Unable to open process.\n");
        return false;
    }

    // Get address of LoadLibraryA function
	loadLibraryA = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

    // Reserve some space in memory for DLL_PATH string
	remoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(DLL_PATH), MEM_COMMIT, PAGE_READWRITE);
    // Write DLL_PATH to memory
	WriteProcessMemory(Proc, (LPVOID)remoteString, DLL_PATH, strlen(DLL_PATH), NULL);

    // Create thread
	HANDLE remoteThread = CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)loadLibraryA, (LPVOID)remoteString, NULL, NULL);

	if (remoteThread)
		WaitForSingleObject(remoteThread, INFINITE);

    // Free reserved memory and close handle
	VirtualFreeEx(Proc, remoteString, strlen(DLL_PATH), MEM_RELEASE);
	CloseHandle(Proc);

	return true;
}

DWORD GetTargetThreadID()
{
	tagPROCESSENTRY32 pe;
	HANDLE snapShot;
	BOOL retval, ProcFound = false;

	snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapShot == INVALID_HANDLE_VALUE)
    {
        printf("ERROR: Unable to take process snapshot.");
        return 0;
    }

	pe.dwSize = sizeof(tagPROCESSENTRY32);

	retval = Process32First(snapShot, &pe);

	while (retval)
	{
        int length = sizeof(processNames) / sizeof(std::string);
        for (int i = 0; i < length; i++)
        {
            if (_stricmp(pe.szExeFile, processNames[i].c_str()) == 0)
            {
                return pe.th32ProcessID;
            }
        }

		retval = Process32Next(snapShot, &pe);
	}
	return 0;
}

void PauseSystem()
{
    printf("\nPress any key to continue...");
    getchar();
}