#pragma once
#include "common.h"
#include "PatternFunctions.h"

extern int* FindGroupVtableOffset();
extern int32* FindConsoleWriteOffset();
extern std::list<std::string> GetAllLines(char* fileName, ...);
extern void InitializeConsole();
extern int GetBuild();

struct ConsoleInfo
{
    int32 HotkeyAddr;
    int32 EnableAddr;
};

extern ConsoleInfo GetConsoleInfo();
