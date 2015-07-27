#include "PatternFunctions.h"
#include "ModuleFunctions.h"

std::list<void*> FindMultiplePatterns(uint8* source, int len, uint8* pattern, int patternLen, std::string patternOptions)
{
    std::list<void*> list;

    if (patternLen != patternOptions.length())
        return list;

    for (int i = 0; i < len; i++)
    {
        int matches = 0;

        if (i + patternLen > len - 1)
            break;

        for (int j = 0; j < patternLen; j++)
        {
            if (patternOptions[j] == 'a' || patternOptions[j] == '?')
            {
                matches++;
                continue;
            }

            if ((patternOptions[j] == 'p' || patternOptions[j] == 'x') && source[i + j] == pattern[j])
            {
                matches++;
                continue;
            }
            break;
        }

        if (matches == patternLen)
            list.push_back((void*)(i + (int)source));
    }

    return list;
}

std::list<void*> FindMultiplePatterns(uint8* pattern, int patternLen, std::string patternOptions)
{
    uint8* source = (uint8*)GetMainModuleAddress();
    int len = GetMainModuleSize();

    return FindMultiplePatterns(source, len, pattern, patternLen, patternOptions);
}

void* FindPattern(uint8* source, int len, uint8* pattern, int patternLen, std::string patternOptions)
{
    if (patternLen != patternOptions.length())
        return 0;

    for (int i = 0; i < len; i++)
    {
        int matches = 0;

        if (i + patternLen > len - 1)
            break;

        for (int j = 0; j < patternLen; j++)
        {
            if (patternOptions[j] == 'a' || patternOptions[j] == '?')
            {
                matches++;
                continue;
            }

            if ((patternOptions[j] == 'p' || patternOptions[j] == 'x') && source[i + j] == pattern[j])
            {
                matches++;
                continue;
            }

            break;
        }

        if (matches == patternLen)

            return (void*)(i + (int)source);
    }

    return 0;
}

void* FindPattern(uint8* pattern, int patternLen, std::string patternOptions)
{
    uint8* source = (uint8*)GetMainModuleAddress();
    int len = GetMainModuleSize();

    return FindPattern(source, len, pattern, patternLen, patternOptions);
}