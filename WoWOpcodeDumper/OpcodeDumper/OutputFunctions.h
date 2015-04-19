#pragma once
#include "Common.h"
#include "sqlite3.h"

enum ConsoleColor
{
    CONSOLE_COLOR_DEFAULT    = 0x0,
    CONSOLE_COLOR_INPUT      = 0x1,
    CONSOLE_COLOR_ECHO       = 0x2,
    CONSOLE_COLOR_ERROR      = 0x3,
    CONSOLE_COLOR_WARNING    = 0x4,
    CONSOLE_COLOR_GLOBAL     = 0x5,
    CONSOLE_COLOR_ADMIN      = 0x6,
    CONSOLE_COLOR_HIGHLIGHT  = 0x7,
    CONSOLE_COLOR_BACKGROUND = 0x8,
    MAX_CONSOLE_COLOR        = 0x9
};

#define DEFAULT_COLOR CONSOLE_COLOR_ADMIN

typedef void (*pConsoleWriteAHandler)(const char *format, ConsoleColor color, ...);
extern pConsoleWriteAHandler ConsoleWriteA;

extern void ConsoleWrite(char* str, ...);
extern void InitializeDebugLoggers(std::string path);

class FileWriter
{
public:
    FileWriter(char* filename, ...);
    FileWriter(char* filename, std::ios::ios_base::openmode openMode, ...);
    ~FileWriter();

    void WriteString(char* str, ...);
private:
    std::ofstream stream;
};

struct CMSGOP;
struct JamData;
class SQLiteWriter
{
public:
    SQLiteWriter(const char* filePath, int clientBuild);
    ~SQLiteWriter();

    void addCMSG(const CMSGOP &cmsgData, int opcode);
    void addSMSG(JamData &jamData, int handler);
private:
    sqlite3* db;
    sqlite3_stmt* CMSGstmt;
    sqlite3_stmt* SMSGstmt;

    // SQLite text Queries
    char* createCMSG =
        "CREATE TABLE CMSG"
        "("
        "opcode INTEGER PRIMARY KEY,"
        "vTable INTEGER,"
        "cliPut INTEGER,"
        "caller INTEGER"
        ");";
    char* createSMSG =
        "CREATE TABLE SMSG"
        "("
        "opcode INTEGER PRIMARY KEY,"
        "ctor INTEGER,"
        "callHandler INTEGER,"
        "handler INTEGER"
        ");";
    char* createVersion =
        "CREATE TABLE Version"
        "("
        "clientBuild INTEGER,"
        "dumperVersion REAL"
        ");";
    char* CMSGInsertQuery =
        "INSERT INTO CMSG "
        "VALUES (?,?,?,?);";
    char* SMSGInsertQuery =
        "INSERT into SMSG "
        "VALUES (?,?,?,?);";
    char* VersionInsertQuery =
        "INSERT into Version "
        "VALUES (?,?);";
};

extern FileWriter* shiftDebugLogger;
extern FileWriter* debugLogger;