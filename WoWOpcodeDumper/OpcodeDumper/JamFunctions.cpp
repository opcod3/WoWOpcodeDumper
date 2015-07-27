#include "JamFunctions.h"

bool GroupVtable::IsJam(__int16 opcode)
{
    pIsJam _isJam = pIsJam(isJam);

    return _isJam(opcode);
}

bool __thiscall GroupVtable::IsInstanceServer(uint32 opcode)
{
    if (!isInstanceServer)
        return false;
    pIsInstanceServer _isInstance = pIsInstanceServer(isInstanceServer);

    return _isInstance(opcode);
}

char GroupVtable::Call(FakeStore* a1, int a2, int a3, int a4, unsigned int a5, FakeStore* a6)
{
    pCall call = (pCall)Exec;
    char result = call((int)a1, a2, a3, a4, a5, (int)a6);

    return result;
}

int* GroupVtable::GetRealCallFunction()
{
    uint8 pattern[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x5D };
    int* ptr = (int*)((int)FindPattern((uint8*)Exec, 0x20, pattern, sizeof(pattern), "paaaap") + 1);

    return (int*)((int)ptr + *ptr + 4);
}

int GroupVtable::GetRealCallFunctionLen()
{
    uint8* func = (uint8*)GetRealCallFunction();

    uint8 pattern[4] = { 0xC9, 0xC2, 0x10, 0x00 };
    int addr = (int)FindPattern(func, 30000, pattern, sizeof(pattern), "pppp");

    if (!addr)
    {
        pattern[0] = 0x5B;
        addr = (int)FindPattern(func, 30000, pattern, sizeof(pattern), "pppp");
    }

    return (addr + sizeof(pattern) - (int)func);
}