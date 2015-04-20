#include "JamFunctions.h"

JamData GetData(uint8* addr)
{
    JamData data;
    data.ctor = 0;
    data.callHandler = 0;

    // Iterates through the client to find the JamData.
    for (int i = 0; i < 50; i++)
    {
        if (addr[i] == 0xE8 && addr[i + 5] == 0x8D)
        {
            // If the address iteration + 6 is LEA and address iteration + 11 is Call
            if (addr[i + 6] == 0x8D && addr[i + 11] == 0xE8)
            {
                // Finds the data based on the address for ctor and call handler.
                JamData data;
                data.ctor = ((uint32)addr + i) + (*(uint32*)(addr + i + 1)) + 5;
                data.callHandler = ((uint32)addr + i + 11) + (*(uint32*)(addr + i + 12)) + 5;
                data.startsAt = ((uint32)addr) + i;
                int additionalLen = 0;

                // LEA
                if (addr[i + 16] == 0x8D)
                {
                    if (addr[i + 17] == 0x8D)
                        additionalLen = 6;
                    else
                        additionalLen = 3;
                    if (addr[i + 16 + additionalLen] == 0xE8)
                        additionalLen += 5;
                }

                data.len = 15 + i + 1 + additionalLen;
                data.addr = (uint32)addr;
                return data;
            }

            // If the address iteration +6 is 0x4D and address iteration + 8 is Call
            if (addr[i + 6] == 0x4D && addr[i + 8] == 0xE8)
            {
                // Finds the data based on the address for ctor and call handler.
                JamData data;
                data.ctor = ((uint32)addr + i) + (*(uint32*)(addr + i + 1)) + 5;
                data.callHandler = ((uint32)addr + i + 8) + (*(uint32*)(addr + i + 9)) + 5;
                data.startsAt = ((uint32)addr) + i;
                int additionalLen = 0;

                // LEA
                if (addr[i + 13] == 0x8D)
                {
                    if (addr[i + 14] == 0x8D)
                        additionalLen = 6;
                    else
                        additionalLen = 3;

                    if (addr[i + 13 + additionalLen] == 0xE8)
                        additionalLen += 5;
                }

                data.len = 12 + i + 1 + additionalLen;
                data.addr = (uint32)addr;
                return data;
            }
        }
    }

    // Cannot obtain address
    assert("Fatal error at getting addr" && false);
    return data;
}

bool GroupVtable::IsJam(__int16 opcode)
{
	pIsJam _isJam = (pIsJam)(isJam);
	return _isJam(opcode);
}

bool __thiscall GroupVtable::IsInstanceServer(uint32 opcode)
{
	if (!isInstanceServer)
		return false;
	pIsInstanceServer _isInstance = (pIsInstanceServer)(isInstanceServer);
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
    uint8 pattern[] = {0xE8, 0x00, 0x00, 0x00, 0x00, 0x5D};
    int* ptr = (int*)((int)FindPattern((uint8*)Exec, 0x20, pattern, sizeof(pattern), "paaaap") + 1);
    return (int*)((int)ptr + *ptr + 4);
}

int GroupVtable::GetRealCallFunctionLen()
{
    uint8* func = (uint8*)GetRealCallFunction();

    uint8 pattern[4] = { 0xC9, 0xC2, 0x10, 0x00};
    int addr = (int)FindPattern(func, 30000, pattern, sizeof(pattern), "pppp");

    if (!addr)
    {
        pattern[0] = 0x5B;
        addr = (int)FindPattern(func, 30000, pattern, sizeof(pattern), "pppp");

    }
    return (addr + sizeof(pattern) - (int)func);
}