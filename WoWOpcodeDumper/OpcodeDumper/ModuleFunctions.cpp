#include "ModuleFunctions.h"

HMODULE GetMainModule()
{
	HMODULE *lphModule;
	int count = 1;
	DWORD cb = sizeof(HMODULE) * count;
	DWORD cbNeeded;

	do
	{
		lphModule = new HMODULE[++count];
		cb = sizeof(HMODULE) * count;
		EnumProcessModulesEx(GetCurrentProcess(), lphModule, cb, &cbNeeded, LIST_MODULES_ALL);
	} while (cbNeeded > cb);

	HMODULE module = lphModule[0];
	delete[] lphModule;
	return module;
}

int GetMainModuleSize()
{
	MODULEINFO mi;
	GetModuleInformation(GetCurrentProcess(), GetMainModule(), &mi, sizeof(mi));
	return mi.SizeOfImage;
}

int GetMainModuleAddress()
{
	return (int)GetMainModule();
}
