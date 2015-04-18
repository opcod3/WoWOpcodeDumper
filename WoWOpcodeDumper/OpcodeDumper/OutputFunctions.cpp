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

SQLiteWriter::SQLiteWriter(const char* filePath)
{
    // Delete DB if it already exists
    DeleteFileA(filePath);

    int dbResult = sqlite3_open_v2(filePath, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
    if (dbResult)
    {
        LOG_DEBUG("Unable to open sqlite database!\nError: %s", sqlite3_errstr(dbResult));
        assert("Failed to open SQlite databas");
    }

    // Create CMSG and SMSG tables in DB
    char* errmsg;
    if (sqlite3_exec(db, createCMSG, NULL, NULL, &errmsg))
    {
        LOG_SHIFT("SQLITE ERROR: %s", errmsg);
        assert("Failed to create CMSG table in SQLite database");
    }

    if (sqlite3_exec(db, createSMSG, NULL, NULL, &errmsg))
    {
        LOG_SHIFT("SQLITE ERROR: %s", errmsg);
        assert("Failed to create SMSG table in SQLite database");
    }

    // Create statements to insert data into tables
    int err;
    if (err = sqlite3_prepare_v2(db, CMSGInsertQuery, -1, &CMSGstmt, NULL))
    {
        LOG_SHIFT("SQLITE ERROR: %s", sqlite3_errstr(err));
        assert("Unable to prepare CMSG insert statement");
    }
    if (err = sqlite3_prepare_v2(db, SMSGInsertQuery, -1, &SMSGstmt, NULL))
    {
        LOG_SHIFT("SQLITE ERROR: %s", sqlite3_errstr(err));
        assert("Unable to prepare SMSG insert statement");
    }
}

SQLiteWriter::~SQLiteWriter()
{
    sqlite3_finalize(CMSGstmt);
    sqlite3_finalize(SMSGstmt);
    sqlite3_close_v2(db);
}

void SQLiteWriter::addCMSG(const CMSGOP &cmsgData, int opcode)
{
    // Bind values to statement
    sqlite3_bind_int(CMSGstmt, 1, opcode);
    sqlite3_bind_int(CMSGstmt, 2, FIX_ADDR(cmsgData.offset));
    sqlite3_bind_int(CMSGstmt, 3, FIX_ADDR(cmsgData.putData));
    sqlite3_bind_int(CMSGstmt, 4, FIX_ADDR(cmsgData.caller));

    // Run statement
    sqlite3_step(CMSGstmt);

    // Reset statement
    sqlite3_reset(CMSGstmt);
    sqlite3_clear_bindings(CMSGstmt);
}

void SQLiteWriter::addSMSG(JamData &jamData, int handler)
{
    sqlite3_bind_int(SMSGstmt, 1, jamData.opcode);
    sqlite3_bind_int(SMSGstmt, 2, FIX_ADDR(jamData.ctor));
    sqlite3_bind_int(SMSGstmt, 3, FIX_ADDR(jamData.callHandler));
    sqlite3_bind_int(SMSGstmt, 4, handler);

    // Run statement
    sqlite3_step(SMSGstmt);

    // Reset Statement
    sqlite3_reset(SMSGstmt);
    sqlite3_clear_bindings(SMSGstmt);
}