#include "common.h"

int Build;
std::string WorkingDir;
std::string ModulePath;
std::list<int> CallHandlerList;
std::list<CallEntry*> CallList;
OpcodeMap opcodeMap;
FileWriter* output = nullptr;
int lastOpcode;

struct CMSGOP
{
	int offset;
	int putData;
	int putOpcode;
};

void FillCallList()
{
	uint8 list[] = { 0xC7, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	std::string options = "xx????????";
	std::list<void*> _list = FindMultiplePatterns(list, sizeof(list), options);

	for (std::list<void*>::const_iterator iter = _list.begin(); iter != _list.end(); iter++)
	{
		CallEntry* callEntry = new CallEntry();
		callEntry->addr = (int*)*(int*)(((int)*iter) + 2);
		callEntry->setValue = *(int*)(((int)*iter) + 6);
		CallList.push_back(callEntry);

	}
}

JamData GetData(uint8* addr)
{
	JamData data;
	data.ctor = 0;
	data.callHandler = 0;

	for (int i = 0; i < 50; i++)
	{
		if (addr[i] == 0xE8 && addr[i + 5] == 0x8D)
		{
			if (addr[i + 6] == 0x8D && addr[i + 11] == 0xE8)
			{
				JamData data;
				data.ctor = ((int)addr + i) + (*(int*)(addr + i + 1)) + 5;
				data.callHandler = ((int)addr + i + 11) + (*(int*)(addr + i + 12)) + 5;
				data.startsAt = ((int)addr) + i;
				int additionalLen = 0;

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
				data.addr = (int)addr;
				return data;
			}

			if (addr[i + 6] == 0x4D && addr[i + 8] == 0xE8)
			{
				JamData data;
				data.ctor = ((int)addr + i) + (*(int*)(addr + i + 1)) + 5;
				data.callHandler = ((int)addr + i + 8) + (*(int*)(addr + i + 9)) + 5;
				data.startsAt = ((int)addr) + i;
				int additionalLen = 0;

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
				data.addr = (int)addr;
				return data;
			}

		}
	}

	assert("Fatal error at getting addr" && false);
	return data;
}

int FindApHandler(int* addr)
{
	for (std::list<CallEntry*>::const_iterator iter = CallList.begin(); iter != CallList.end(); iter++)
	{
		int* thisAddr = (*iter)->addr;

		if (thisAddr == addr)
			return (int)(*iter)->setValue;
	}
	return 0;
}

void CalledHandler(int addr)
{
	JamData* data = opcodeMap[addr];
	data->opcode = lastOpcode;
	int callAddr = (int)data->GetCallHandlerAddr();
	CallHandlerList.push_back(callAddr);
	return;
}

DWORD RemoveProtect(char* ptr, int len)
{
	DWORD oldProtect;
	VirtualProtect(ptr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
	return oldProtect;
}

DWORD RemoveProtectDataStore(char* ptr, int len)
{
	DWORD oldProtect;
	VirtualProtect(ptr, len, PAGE_READWRITE, &oldProtect);
	return oldProtect;
}

void FillHandler(JamData& data)
{
	uint8 buffer[60];
	memset(buffer, (char)0x90, sizeof(buffer));
	buffer[0] = 0x68; // push
	memcpy(&buffer[1], &data.addr, sizeof(int));
	buffer[5] = 0xE8; // call
	int memory = ((int)&CalledHandler - (int)(data.addr + 5 + 5));
	memcpy(&buffer[6], &memory, sizeof(int));
	RemoveProtect((char*)data.addr, data.len);
	memcpy((char*)data.addr, &buffer[0], data.len);
}

bool IsCMSG(void* argAddr)
{
	int* bs = (int*)(((int)argAddr) - 4);
	int address = *bs;

	if (address < GetMainModuleAddress() || address >(GetMainModuleAddress() + GetMainModuleSize()))
		return false;

	uint8* addr = (uint8*)address;
	if (addr[9] == 0x68 && (addr[0xE] == 0xE8 || addr[0xE] == 0x6A))
		return true;
	return false;
}

void main()
{
	int initialTicks = GetTickCount();
	Build = GetBuild();

	char buildPath[32];
	printf(buildPath, "\\Build_%i", Build);
	WorkingDir.append(buildPath);

	InitializeDebugLoggers(WorkingDir);

	output = new FileWriter("%s\\%i_opcode_dump.txt", WorkingDir.c_str(), Build);

	InitializeConsole();

	ConsoleWrite("Dumper Attached");

	ConsoleWrite("Detected Build: %i", Build);
	LOG_DEBUG("Detected Build: %i", Build);

	FillCallList();

	int32* addr = FindGroupVtableOffset();
	LOG_DEBUG("GroupVtableListPointer: 0x%08X (0x%08X)", addr, addr);
	assert("Wrong GroupVtableListPointer" && addr);

	uint8 groupCount = addr[1];

	std::vector<GroupVtable*> groups;
	groups.resize(groupCount);

	LOG_DEBUG("Detected %i JAM vtables", groupCount);

	std::list<std::vector<uint8>> possibleJamPatterns;
	possibleJamPatterns.push_back({ 0xFF, 0x73, 0x0C });
	possibleJamPatterns.push_back({ 0xFF, 0x75, 0x0C });

	LOG_DEBUG("Dumping Groups");
	for (int i = 0; i < groupCount; i++)
	{
		RawGroupVtable* rawTable = (RawGroupVtable*)((int32**)*addr)[i];
		GroupVtable* vtable = new GroupVtable(rawTable);

		LOG_DEBUG("[%i] %s - 0x%08X", i, vtable->name, rawTable);
		LOG_DEBUG("[%i] Call function at %08X size %i", i, vtable->ReallCall, vtable->ReallCallSize);
		groups[i] = vtable;

		for (std::list<std::vector<uint8>>::const_iterator itr = possibleJamPatterns.begin(); itr != possibleJamPatterns.end(); itr++)
		{
			uint8 pattern[3];
			memcpy(pattern, &(*itr)[0], sizeof(pattern));
			std::list<void*> list = FindMultiplePatterns((uint8*)vtable->ReallCall, vtable->ReallCallSize, pattern, sizeof(pattern), "ppp");

			for (std::list<void*>::const_iterator iter = list.begin(); iter != list.end(); iter++)
			{
				JamData* data = new JamData(GetData((uint8*)*iter));
				data->table = vtable;
				opcodeMap.insert(std::make_pair((int)*iter, data));
				FillHandler(*data);
			}
		}
	}

	// Fake Pointer
	FakeStore* fakeStore = new FakeStore();
	memset(fakeStore, 0, sizeof(FakeStore));
	RemoveProtectDataStore((char*)fakeStore, sizeof(FakeStore));

	for (uint16 opcode = 0; opcode < 0x1FFF; opcode++)
	{
		lastOpcode = opcode;

		for (int i = 0; i < groupCount; i++)
		{
			GroupVtable* vtable = groups[i];

			if (vtable->IsJam(opcode))
			{
				vtable->Call(fakeStore, 0, 0, 0, opcode, fakeStore);
				break;
			}
		}
	}

	delete fakeStore;
	std::unordered_map<int, JamData*> newOpcodeMap;
	for (std::unordered_map<int, JamData*>::const_iterator iter = opcodeMap.begin(); iter != opcodeMap.end(); iter++)
		if (newOpcodeMap.find(iter->second->opcode) == newOpcodeMap.end())
			newOpcodeMap[iter->second->opcode] = iter->second;
		else
			ConsoleWrite("duplicate %X %X", newOpcodeMap.find(iter->second->opcode)->second->addr, iter->second->addr);

	for (int i = 0; i < MAX_OPCODE; i++)
	{
		std::unordered_map<int, JamData*>::const_iterator iter = newOpcodeMap.find(i);

		if (iter == newOpcodeMap.end())
			continue;

		JamData* data = iter->second;
		int addr = (int)data->GetCallHandlerAddr();
		int* handler = (int*)FindApHandler((int*)addr);

		if (handler)
			data->table->trueOpCount++;
		data->table->opCount++;

		output->WriteString("0x%04X %i %06X %06X %06X SMSG %s %i %s%s", data->opcode, data->opcode, data->ctor, data->callHandler, handler, data->table->name, data->table->IsInstanceServer(data->opcode), data->opcodeName.c_str(), handler ? "" : " - Fake Opcode");
	}

	uint8 patternBuffer[] = { 0x55, 0x8b, 0xec, 0x8b, 0x45, 0x08, 0x83, 0x60, 0x04, 0x00, 0x83, 0x60, 0x08, 0x00, 0xc7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5d, 0xc2, 0x04, 0x00 };
	std::string patternOp = "ppppppppppppppppaaaapppp";
	uint8 secondBuff[4];
	int newAddr = (int)FindPattern(patternBuffer, sizeof(patternBuffer), patternOp);
	memcpy(secondBuff, (uint8*)&newAddr, sizeof(secondBuff));
	std::list<void*> cmsgOpList = FindMultiplePatterns((uint8*)(GetMainModuleAddress() + 0x800000), GetMainModuleSize() - 0x800000, secondBuff, sizeof(secondBuff), "pppp");

	std::unordered_map<int, CMSGOP> cmsgMap;
	for (std::list<void*> ::const_iterator iter = cmsgOpList.begin(); iter != cmsgOpList.end();)
	{
		int* bs = (int*)(((int)*iter) - 4);
		int address = *bs;

		if (address < GetMainModuleAddress() || address >(GetMainModuleAddress() + GetMainModuleSize()))
		{
			iter = cmsgOpList.erase(iter);
			continue;
		}

		uint8* addr = (uint8*)address;
		if ((addr[9] == 0x68 && addr[0xE] == 0xE8) || (addr[9] == 0x6A && addr[0xB] == 0xE8))
		{
			CMSGOP op;
			op.offset = ((int)*iter) - 12;
			op.putData = ((int)*iter) - 8;
			op.putOpcode = ((int)*iter) - 4;
			int opcode;
			if (addr[9] == 0x6A)
				opcode = addr[0xA];
			else
				memcpy((uint8*)&opcode, &(addr[0xA]), sizeof(int));
			cmsgMap[opcode] = op;
			iter++;
			continue;
		}

		iter = cmsgOpList.erase(iter);
		continue;
	}

	for (int i = 0; i < MAX_OPCODE; i++)
	{
		std::unordered_map<int, CMSGOP>::const_iterator iter = cmsgMap.find(i);

		if (iter == cmsgMap.end())
			continue;

		output->WriteString("0x%04X %i %X %X CMSG", iter->first, iter->first, iter->second.offset, *(int*)iter->second.putData);
	}

	int totalCount = 0;
	int trueCount = 0;

	for (int i = 0; i < groupCount; i++)
	{
		GroupVtable* group = groups[i];
		totalCount += group->opCount;
		trueCount += group->trueOpCount;
		ConsoleWrite("Group: \"%s\" has %i opcodes (%i fake, %i total)", group->name, group->trueOpCount, group->opCount - group->trueOpCount, group->opCount);
	}

	ConsoleWrite("Found %i SMSG opcodes (%i fake, %i total)", trueCount, totalCount - trueCount, totalCount);
	ConsoleWrite("Found %i CMSG opcodes", cmsgOpList.size());
	ConsoleWrite("Execution Took: %i MS", GetTickCount() - initialTicks);
	ConsoleWrite("Shutting down in %i seconds", SHUTDOWN_TIMER);

	delete output;
	delete debugLogger;
	delete shiftDebugLogger;

	for (int i = 0; i < groupCount; i++)
		delete groups[i];

	Sleep(SHUTDOWN_TIMER * 1000);
	exit(0);
}

std::string GetDirectory(std::string filename)
{
	std::string directory = "";
	const size_t last_slash_idx = filename.rfind('\\');
	if (std::string::npos != last_slash_idx)
	{
		directory = filename.substr(0, last_slash_idx);
	}
	return directory;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		char module[2024];
		//currentModule = hModule;
		GetModuleFileNameA(hModule, module, 2024);
		WorkingDir = GetDirectory(module);
		ModulePath = WorkingDir;
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&main, 0, 0, 0);

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

