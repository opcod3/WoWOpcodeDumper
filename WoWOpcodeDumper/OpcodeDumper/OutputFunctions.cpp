#include "OutputFunctions.h"

pConsoleWriteAHandler ConsoleWriteA;
FileWriter* shiftDebugLogger;
FileWriter* debugLogger;

void ConsoleWrite(char* str, ...)
{
    va_list va;
    va_start(va, str);
    char finalStr[512];
    vsprintf(finalStr, str, va);
    va_end(va);

    ConsoleWriteA(finalStr, DEFAULT_COLOR);
}

FileWriter::FileWriter(char* filename, ...)
{
    va_list va;
    va_start(va, filename);

    char finalName[512];
    vsprintf(finalName, filename, va);
    va_end(va);

    stream = std::ofstream(finalName, std::ios::out | std::ios::trunc);
    assert("Unable to create FileWriter" && stream.is_open());
}

FileWriter::FileWriter(char* filename, std::ios::ios_base::openmode openMode, ...)
{
    va_list va;
    va_start(va, filename);
    char finalName[512];
    vsprintf(finalName, filename, va);
    va_end(va);

    stream = std::ofstream(finalName, openMode);
    assert("Unable to create FileWriter" && stream.is_open());
}

void FileWriter::WriteString(char* str, ...)
{
    va_list va;
    va_start(va, str);
    char finalStr[512];
    vsprintf(finalStr, str, va);
    va_end(va);

    sprintf(finalStr, "%s\n", finalStr);
    stream.write(finalStr, strlen(finalStr));
    stream.flush();
}

FileWriter::~FileWriter()
{
    stream.flush();
    stream.close();
}

void InitializeDebugLoggers(std::string path)
{
    if (CreateDirectoryA(path.c_str(), 0) || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        shiftDebugLogger = new FileWriter("%s\\%s", path.c_str(), FILENAME_SHIFT_DEBUG);
        debugLogger = new FileWriter("%s\\%s", path.c_str(), FILENAME_DEBUG);
    }
    else
        assert("Unable to create output directory");
}