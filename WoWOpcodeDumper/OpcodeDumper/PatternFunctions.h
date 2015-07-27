#pragma once
#include "Common.h"

extern std::list<void*> FindMultiplePatterns(uint8* source, int len, uint8* pattern, int patternLen, std::string patternOptions);
extern std::list<void*> FindMultiplePatterns(uint8* pattern, int patternLen, std::string patternOptions);
extern void* FindPattern(uint8* source, int len, uint8* pattern, int patternLen, std::string patternOptions);
extern void* FindPattern(uint8* pattern, int patternLen, std::string patternOptions);
