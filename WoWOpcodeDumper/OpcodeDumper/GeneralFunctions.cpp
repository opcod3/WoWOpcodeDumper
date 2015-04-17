#include "GeneralFunctions.h"

int* FindGroupVtableOffset()
{
	uint8 pattern[] = { 0xF6, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, // test    byte ptr dword_xxxxxxx, 1
		0x75, 0x00,                               // jnz     short loc_xxxxxx					    	
		0x83, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, // or      dword_xxxxxxx, 1
		0x33, 0xc0,                               // xor     eax, eax
		0x68, 0x00, 0x00, 0x00, 0x00,             // push    offset sub_xxxxxx
		0xA3, 0x00, 0x00, 0x00, 0x00,             // mov     dword_xxxxxxx, eax
		0xA3, 0x00, 0x00, 0x00, 0x00,             // mov     dword_xxxxxxx, eax
		0xA3, 0x00, 0x00, 0x00, 0x00,             // mov     dword_xxxxxxx, eax
		0xE8, 0x00, 0x00, 0x00, 0x00,             // call    sub_xxxxxx
		0x59,                                     // pop     ecx
		0xB8, 0x00, 0x00, 0x00, 0x00, 0xC3        // mov     eax, offset dword_xxxxxxx
	};

	std::string patternOptions = "xx?????x?xx?????xxx????x????x????x????x????xx????x";
	return (int*)*(int*)(((int)FindPattern(pattern, sizeof(pattern), patternOptions)) + 45);
}

int32* FindConsoleWriteOffset()
{
	uint8 pattern[] = { 0x55,                                   // push    ebp
		0x8B, 0xEC,                             // mov     ebp, esp
		0x8B, 0x45, 0x08,                       // mov     eax, [ebp+arg_0]
		0x81, 0xEC, 0x00, 0x00, 0x00, 0x00,     // sub     esp, 400h
		0x85, 0xC0,                             // test    eax, eax
		0x74, 0x00,                             // jz      short locret_xxxxxx
		0x80, 0x38, 0x00,                       // cmp     byte ptr [eax], 0
		0x74, 0x00,                             // jz      short locret_xxxxxx
		0x8D, 0x4D, 0x10,                       // lea     ecx, [ebp+arg_8]
		0x51,                                   // push    ecx
		0x50,                                   // push    eax
		0x8D, 0x85, 0x00, 0xFC, 0xFF, 0xFF,     // lea     eax, [ebp+var_400]
		0x68, 0x00, 0x00, 0x00, 0x00,           // push    400h
		0x50,                                   // push    eax
		0xE8, 0x00, 0x00, 0x00, 0x00,           // call    sub_xxxxxx
		0xFF, 0x75, 0x0C,                       // push    [ebp+arg_4]
		0x8D, 0x85, 0x00, 0xFC, 0xFF, 0xFF,     // lea     eax, [ebp+var_400]
		0x50,                                   // push    eax
		0xE8, 0x00, 0x00, 0x00, 0x00,           // call    sub_xxxxxx
		0x83, 0xC4, 0x18,                       // add     esp, 18h
		0xC9,                                   // leav
		0xC3                                    // retn
	};

	std::string patternOptions = "xxxxxxxx????xxx?xxxx?xxxxxxxxxxxx????xx????xxxxxxxxxxx????xxxxx";
	return (int32*)FindPattern(pattern, sizeof(pattern), patternOptions);
}

ConsoleInfo GetConsoleInfo()
{
	uint8 pattern[] = { 0x55,
		0x8b, 0xec,
		0x8b, 0x45, 0x08,
		0x8b, 0x00, 0x3b,
		0x05, 0x00, 0x00, 0x00, 0x00,
		0x75, 0x00,
		0x83, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x74, 0x00, 0x33,
		0xc0,
		0x5d,
		0xc3
	};

	ConsoleInfo consoleInfo;
	std::string patternOptions = "xxxxxxxxxx????x?xx?????x?xxxx";

	int32 addr = (int32)FindPattern(pattern, sizeof(pattern), patternOptions);
	consoleInfo.HotkeyAddr = *(int*)(addr + 0xA);
	consoleInfo.EnableAddr = *(int*)(addr + 0x12);
	return consoleInfo;
}

std::list<std::string> GetAllLines(char* fileName, ...)
{
	va_list va;
	va_start(va, fileName);
	char finalStr[2048];
	vsprintf_s(finalStr, fileName, va);
	va_end(va);

	std::list<std::string> lines;
	std::ifstream file;
	std::string str;

	file.open(finalStr);
	while (std::getline(file, str))
		lines.push_back(str);

	return lines;
}

void InitializeConsole()
{
	ConsoleInfo info = GetConsoleInfo();
	int ConsoleWriteAddr = (int)FindConsoleWriteOffset();

	LOG_DEBUG("ConsoleHokeyAddr: 0x%08X (0x%08X)", info.HotkeyAddr, info.HotkeyAddr);
	LOG_DEBUG("ConsoleEnabledAddr: 0x%08X (0x%08X)", info.EnableAddr, info.EnableAddr);
	LOG_DEBUG("ConsoleWriteAAddr: 0x%08X (0x%08X)", ConsoleWriteAddr, ConsoleWriteAddr);

	assert("Missing ConsoleHotkeyAddr" && info.HotkeyAddr);
	assert("Missing ConsoleEnabledAddr" && info.EnableAddr);
	assert("Missing ConsoleWriteAddr" && ConsoleWriteAddr);

	ConsoleWriteA = pConsoleWriteAHandler(ConsoleWriteAddr);

	*(int32*)(info.HotkeyAddr) = 515; // TAB;
	LOG_DEBUG("Console hot-key set to TAB (515)");

	*(int32*)(info.EnableAddr) = 1;
	LOG_DEBUG("Console Enabled");
}

int GetBuild()
{
	uint8 buffer[] = { 0x28, 0x62, 0x75, 0x69, 0x6c, 0x64, 0x20 };
	std::string patternOptions = "ppppppp";
	void* result = FindPattern(buffer, sizeof(buffer), patternOptions);

	char buildBuffer[6];
	memset(buildBuffer, 0x0, sizeof(buildBuffer));
	memcpy(buildBuffer, (char*)((int)result + 7), 5);
	return atoi(buildBuffer);
}