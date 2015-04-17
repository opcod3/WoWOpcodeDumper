#pragma once
#include "Common.h"

struct CallEntry
{
    int* addr;
    int setValue;
};

extern std::list<CallEntry*> CallList;
extern std::list<int> CallHandlerList;

