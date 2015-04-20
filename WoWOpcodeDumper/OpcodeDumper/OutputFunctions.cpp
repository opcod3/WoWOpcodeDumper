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
        debugLogger = new FileWriter("%s\\%s", path.c_str(), FILENAME_DEBUG);
    }
    else
        assert("Unable to create output directory");
}

SQLiteWriter::SQLiteWriter(const char* filePath, int clientBuild)
{
    // Delete DB if it already exists
    DeleteFileA(filePath);

    int dbResult = sqlite3_open_v2(filePath, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
    if (dbResult)
    {
        LOG_DEBUG("Unable to open sqlite database!\nError: %s", sqlite3_errstr(dbResult));
        assert("Failed to open SQlite databas");
    }

    // Create Version, CMSG and SMSG tables in DB
    char* errmsg;
    if (sqlite3_exec(db, createCMSG, NULL, NULL, &errmsg))
    {
        LOG_DEBUG("SQLITE ERROR(createCMSG): %s", errmsg);
        assert("Failed to create CMSG table in SQLite database");
    }

    if (sqlite3_exec(db, createSMSG, NULL, NULL, &errmsg))
    {
        LOG_DEBUG("SQLITE ERROR(createSMSG): %s", errmsg);
        assert("Failed to create SMSG table in SQLite database");
    }
    if (sqlite3_exec(db, createVersion, NULL, NULL, &errmsg))
    {
        LOG_DEBUG("SQLITE ERROR(createVersion): %s", errmsg);
        assert("Failed to create Version table in SQLite database");
    }

    // Create statements to insert data into tables
    int err;
    if (err = sqlite3_prepare_v2(db, CMSGInsertQuery, -1, &CMSGstmt, NULL))
    {
        LOG_DEBUG("SQLITE ERROR(CMSGstmt): %s", sqlite3_errstr(err));
        assert("Unable to prepare CMSG insert statement");
    }
    if (err = sqlite3_prepare_v2(db, SMSGInsertQuery, -1, &SMSGstmt, NULL))
    {
        LOG_DEBUG("SQLITE ERROR(SMSGstmt): %s", sqlite3_errstr(err));
        assert("Unable to prepare SMSG insert statement");
    }

    // Write version info to the Version table
    sqlite3_stmt *versionStmt;
    if (err = sqlite3_prepare_v2(db, VersionInsertQuery, -1, &versionStmt, NULL))
    {
        LOG_DEBUG("SQLITE ERROR(versionStmt): %s", sqlite3_errstr(err));
        assert("Unable to prepare Version create statement");
    }
    
    // Bind data to version statement
    sqlite3_bind_int(versionStmt, 1, clientBuild);
    sqlite3_bind_double(versionStmt, 2, DB_VERSION);

    // Run statement
    sqlite3_step(versionStmt);

    // Delete statement
    sqlite3_finalize(versionStmt);
}

SQLiteWriter::~SQLiteWriter()
{
    sqlite3_finalize(CMSGstmt);
    sqlite3_finalize(SMSGstmt);
    sqlite3_close_v2(db);
}

void SQLiteWriter::addCMSG(std::unordered_map<uint32, CMSGOP>::const_iterator &opcodeData)
{
    // Bind values to statement
    sqlite3_bind_int(CMSGstmt, 1, opcodeData->first);
    sqlite3_bind_int(CMSGstmt, 2, FIX_ADDR(opcodeData->second.offset));
    sqlite3_bind_int(CMSGstmt, 3, FIX_ADDR(opcodeData->second.putData));
    sqlite3_bind_int(CMSGstmt, 4, FIX_ADDR(opcodeData->second.caller));

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
    sqlite3_bind_int(SMSGstmt, 4, FIX_ADDR(handler));

    // Run statement
    sqlite3_step(SMSGstmt);

    // Reset Statement
    sqlite3_reset(SMSGstmt);
    sqlite3_clear_bindings(SMSGstmt);
}