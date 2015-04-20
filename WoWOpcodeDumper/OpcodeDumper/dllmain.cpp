#include "common.h"

int Build;
std::string WorkingDir;
std::string ModulePath;
std::list<int> CallHandlerList;
std::list<CallEntry*> CallList;
OpcodeMap opcodeMap;
int lastOpcode;

#ifndef NO_TEXT_DUMP
FileWriter* output = nullptr;
#endif

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

// Generate Handler data. 
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

uint8* getCMSGCaller(uint8* vTable)
{
    uint8 caller1[] = { 0x8B, 0xC1 };       // mov eax, ecx
    uint8 caller2[] = { 0x56, 0x8B, 0xF1 }; // push esi | mov esi, ecx
    uint8 caller3[] = { 0x55, 0x85, 0xEC }; // push ebp | mov ebp, esp

    // Convert vTable pointer to byte array
    uint8 vTableAddr[4];
    memcpy(vTableAddr, &vTable, sizeof(vTable));

    uint8* xRef = (uint8*)FindPattern(vTableAddr, sizeof(vTableAddr), "pppp");

    // Get start of the function
    // TO-DO optimize this
    uint8* callerFunc = xRef - 1;
    while (true)
    {
        if (callerFunc[0] == caller1[0])
        {
            if (callerFunc[1] == caller1[1])
                break;     
        }
        
        if (callerFunc[0] == caller2[0])
        {
            if (callerFunc[1] == caller2[1])
                if (callerFunc[2] == caller2[2])
                    break;
        }

        if (callerFunc[0] == caller3[0])
        {
            if (callerFunc[1] == caller3[1])
                if (callerFunc[2] == caller3[2])
                    break;          
        }

        callerFunc -= 1;
    }

    return callerFunc;
}

void main()
{
    int initialTicks = GetTickCount();
    Build = GetBuild();

    // Append build number
    char buildPath[32];
    sprintf(buildPath, "\\Build_%i", Build);
    WorkingDir.append(buildPath);

    // Need to do this before output file, otherwise crash
    InitializeDebugLoggers(WorkingDir);

    // Init output file
#ifndef NO_TEXT_DUMP
    output = new FileWriter("%s\\%i_opcode_dump.txt", WorkingDir.c_str(), Build);
#endif

    InitializeConsole();

    ConsoleWrite("Dumper Attached");

    ConsoleWrite("Detected Build: %i", Build);
    LOG_DEBUG("Detected Build: %i", Build);

    // Open SQLite DB writer
    SQLiteWriter dbWriter((WorkingDir.append("\\Dump.db").c_str()), Build);

    FillCallList();

    int32* addr = FindGroupVtableOffset();
    LOG_DEBUG("GroupVtableListPointer: 0x%08X (0x%08X)", FIX_ADDR(addr), addr);
    assert("Wrong GroupVtableListPointer" && addr);

    uint8 groupCount = addr[1];

    std::vector<GroupVtable*> groups;
    groups.resize(groupCount);

    LOG_DEBUG("Detected %i JAM vtables", groupCount);

    // Patterns for JAM opcodes.
    std::list<std::vector<uint8>> possibleJamPatterns;
    possibleJamPatterns.push_back({ 0xFF, 0x73, 0x0C });
    possibleJamPatterns.push_back({ 0xFF, 0x75, 0x0C });

	// SMSG
    LOG_DEBUG("Dumping Groups");
    for (int i = 0; i < groupCount; i++)
    {
        RawGroupVtable* rawTable = (RawGroupVtable*)((int32**)*addr)[i];
        GroupVtable* vtable = new GroupVtable(rawTable);

        LOG_DEBUG("[%i] %s - 0x%08X", i, vtable->name, FIX_ADDR(rawTable));
        LOG_DEBUG("[%i] Call function at 0x%08X size %i", i, FIX_ADDR(vtable->ReallCall), vtable->ReallCallSize);
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

    for (uint16 opcode = 0; opcode < MAX_OPCODE; opcode++)
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
    for ( std::unordered_map<int, JamData*>::const_iterator iter = opcodeMap.begin(); iter != opcodeMap.end(); iter++)
        if (newOpcodeMap.find(iter->second->opcode) == newOpcodeMap.end())
            newOpcodeMap[iter->second->opcode] = iter->second;
        else
        ConsoleWrite("duplicate %X %X", newOpcodeMap.find(iter->second->opcode)->second->addr, iter->second->addr);

#ifndef NO_TEXT_DUMP
    output->WriteString("----------------------------------------------------------------------------");
    output->WriteString(" Hex  |  Dec  |  Ctor  | CallHandler |  Handler | Type |  JamGroup  | Name |");
    output->WriteString("----------------------------------------------------------------------------");
#endif

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

#ifndef NO_TEXT_DUMP
        output->WriteString("0x%04X   %04i   %08X   %08X   %08X   SMSG   %s  %i  %s%s", data->opcode, data->opcode, FIX_ADDR(data->ctor), FIX_ADDR(data->callHandler), FIX_ADDR((int)handler), data->table->name, data->table->IsInstanceServer(data->opcode), data->opcodeName.c_str(), handler ? " - Naming Coming Soon" : " - Fake Opcode");
#endif

        dbWriter.addSMSG(*data, FIX_ADDR((int)handler));
    }

	// CMSG
    //
    // Pattern of the destructor used in CMSGs
    uint8 dtorPattern[] = {   0x55,                                // push     ebp
		                      0x8B, 0xEC,                          // mov      ebo, esp
		                      0x8B, 0x45, 0x08,                    // mov      eax, [ebp+a1]
		                      0x83, 0x60, 0x04, 0x00,              // and      dword ptr [eax+4], 0
		                      0x83, 0x60, 0x08, 0x00,              // and      dword ptr [eax+8], 0
		                      0xC7, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov      dword ptr [eax], offset unk_XXXXXX
		                      0x5D,                                // pop      ebp
		                      0xC2, 0x04, 0x00                     // retn     4
                            };

    std::string patternOp = "ppppppppppppppppaaaapppp";

    // Turn dtor address to a byte array
    uint8 dtorAddr[4];
    uint32* dtorAddr_i = (uint32*)FindPattern(dtorPattern, sizeof(dtorPattern), patternOp);
    memcpy(dtorAddr, &dtorAddr_i, sizeof(dtorAddr));

    // Find all occurrences of the dtor 
    // 0x800000 seems to be a random/estimated value before which vTables don't occur
    std::list<void*> cmsgOpList = FindMultiplePatterns((uint8*)(GetMainModuleAddress() + 0x800000), GetMainModuleSize() - 0x800000, dtorAddr, sizeof(dtorAddr), "pppp");

    // Go through all occurrences of the dtor
    std::unordered_map<uint32, CMSGOP> cmsgMap;
    for (std::list<void*> ::const_iterator dtor = cmsgOpList.begin(); dtor != cmsgOpList.end(); )
	{
        // Get address of cliPutWithMsgId
        uint32* cliPutWithMsgId = (uint32*)(((uint32)*dtor) - 4);

        // Check if cliPutWithMsgId is within the main module
        if ((int)*cliPutWithMsgId < GetMainModuleAddress() || (int)*cliPutWithMsgId > (GetMainModuleAddress() + GetMainModuleSize()))
        {
            dtor = cmsgOpList.erase(dtor);
            continue;
        }
    
        // Check if cliPutWithMsgId pushes opcode as an argument
        uint8* cliPutWithMsgIdAddr = (uint8*)*cliPutWithMsgId;
        if ((cliPutWithMsgIdAddr[9] == 0x68 && cliPutWithMsgIdAddr[0xE] == 0xE8) || // push [OPCODE](DWORD) call [CDataStore__PutUInt32]
            (cliPutWithMsgIdAddr[9] == 0x6A && cliPutWithMsgIdAddr[0xB] == 0xE8))   // push [OPCODE](BYTE) call [CDataStore__PutUInt32]
        {
            // Get opcode data from vTable
            CMSGOP op;
            op.offset = ((uint32)*dtor) - 12;
            op.putData = ((uint32)*dtor) - 8;
            op.putOpcode = ((uint32)*dtor) - 4;
            op.caller = (uint32)getCMSGCaller((uint8*)op.offset);
            uint32 opcode;
            if (cliPutWithMsgIdAddr[9] == 0x6A)
                opcode = cliPutWithMsgIdAddr[0xA];
            else
                memcpy((uint8*)&opcode, &(cliPutWithMsgIdAddr[0xA]), sizeof(uint32));
            cmsgMap[opcode] = op;
            dtor++;
            continue;
        }

        dtor = cmsgOpList.erase(dtor);
        continue;
    }

    // Write table header for CMSG
#ifndef NO_TEXT_DUMP
    output->WriteString("---------------------------------------------");
    output->WriteString("  Hex  |  Dec  |  VTable  |  CliPut  | Type |");
    output->WriteString("---------------------------------------------");
#endif
    
    // Log all CMSG opcodes to text file
    for (int i = 0; i < 0x1FFF; i++)
    {
        std::unordered_map<uint32, CMSGOP>::const_iterator cmsg = cmsgMap.find(i);

        if (cmsg == cmsgMap.end())
            continue;

#ifndef NO_TEXT_DUMP
        output->WriteString("0x%04X   %04i   %08X   %08X   CMSG", cmsg->first, cmsg->first, FIX_ADDR(cmsg->second.offset), FIX_ADDR(*(uint32*)cmsg->second.putData));
#endif
        dbWriter.addCMSG(cmsg);
    }

    int totalCount = 0;
    int trueCount = 0;

    // Iterate through the groups and opcodes within them.
    for (int i = 0; i < groupCount; i++)
    {
        GroupVtable* group = groups[i];
        totalCount += group->opCount;
        trueCount += group->trueOpCount;
        ConsoleWrite("JAMGroup: \"%s\" has %i opcodes (%i fake, %i total)", group->name, group->trueOpCount, group->opCount - group->trueOpCount, group->opCount);
    }

    ConsoleWrite("%i -- SMSG opcodes were dumped (%i fake, %i total)", trueCount, totalCount - trueCount, totalCount);
    ConsoleWrite("%i -- CMSG opcodes were dumped", cmsgOpList.size());
    ConsoleWrite("Execution Took: %i MS", GetTickCount() - initialTicks);
    ConsoleWrite("Shutting down WoW in %i seconds", SHUTDOWN_TIMER);

    // Flush & Close loggers & output ~
#ifndef NO_TEXT_DUMP
    delete output;
#endif
    delete debugLogger;

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
    {
        char module[2024];
        //currentModule = hModule;
        GetModuleFileNameA(hModule, module, 2024);
        WorkingDir = GetDirectory(module);
        ModulePath = WorkingDir;
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&main, 0, 0, 0);
    }
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
};
