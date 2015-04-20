#include "Windows.h"
#include "GeneralFunctions.h"
#include "JamFunctions.h"
#include "ModuleFunctions.h"
#include "NamingFunctions.h"
#include "OutputFunctions.h"
#include "PatternFunctions.h"

#include <assert.h>
#include <fstream>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include <memory>
#include <Psapi.h>
#include <stdarg.h>

typedef __int64            int64;
typedef __int32            int32;
typedef __int16            int16;
typedef char                int8;
typedef unsigned __int64  uint64;
typedef unsigned __int32  uint32;
typedef unsigned __int16  uint16;
typedef unsigned char      uint8;

extern std::string WorkingDir;
extern std::string ModulePath;
extern int Build;

#define MAX_OPCODE 0x1FFF
#define SHUTDOWN_TIMER 5
#define FILENAME_DEBUG "debug.txt"
#define FILENAME_SHIFT_DEBUG "shift_debug.txt"

#define LOG_DEBUG debugLogger->WriteString
#define LOG_SHIFT shiftDebugLogger->WriteString
#define FIX_ADDR(addr)(addr - (int)GetModuleHandle(NULL) + 0x400000)

// Comment this out to have the dll dump opcodes to a text file
#define NO_TEXT_DUMP

#define DB_VERSION 1.0

#pragma once
