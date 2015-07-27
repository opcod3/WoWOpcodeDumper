#include "ModuleFunctions.h"

int GetMainModuleSize()
{
    MODULEINFO mi;
    GetModuleInformation(GetCurrentProcess(), GetMainModule(), &mi, sizeof(mi));

    return mi.SizeOfImage;
}

int GetMainModuleAddress()
{
    return int(GetMainModule());
}