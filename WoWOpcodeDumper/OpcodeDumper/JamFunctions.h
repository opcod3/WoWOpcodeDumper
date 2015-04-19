#pragma once
#include "Common.h"
#include "PatternFunctions.h"

struct FakeStore
{
	uint8 buffer[0x1000];
};

typedef bool(__cdecl *pIsJam)(int16);
typedef bool(__cdecl *pIsInstanceServer)(int32);
typedef char(__cdecl *pCall)(int32, int32, int32, int32, int16, int32);

struct RawGroupVtable
{
	uint8* name;
	int isJam;
	int unused;
	int unused2;
	int isInstanceServer;
	int Exec;
	int unk;
};

struct GroupVtable : RawGroupVtable
{
	GroupVtable(RawGroupVtable* rawTable)
	{
		memcpy(this, rawTable, sizeof(RawGroupVtable));
		ReallCall = GetRealCallFunction();
		ReallCallSize = GetRealCallFunctionLen();
		opCount = 0;
		trueOpCount = 0;
	}

	int* ReallCall;
	int ReallCallSize;
	int opCount;
	int trueOpCount;

	bool IsJam(__int16 opcode);
	bool __thiscall IsInstanceServer(uint32 opcode);
	char Call(FakeStore* a1, int a2, int a3, int a4, unsigned int a5, FakeStore* a6);

private:
	int* GetRealCallFunction();
	int GetRealCallFunctionLen();
};

struct JamData
{
	int ctor;
	int callHandler;
	int startsAt;
	int len;
	int addr;
	int opcode;
	std::string opcodeName;

	int* GetCallHandlerAddr() const { return (int*)*(int*)(callHandler + 1); };
	GroupVtable* table;
};

struct CMSGOP
{
    uint8 offset;
    uint8 putData;
    uint8 putOpcode;
    uint8 caller;
};

typedef std::unordered_map<int, JamData*> OpcodeMap;