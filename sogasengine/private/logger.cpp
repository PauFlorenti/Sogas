#include "logger.h"

#include <cstdarg>
#include <windows.h>

#define LOG_BUFFER_SIZE 1024 * 8

void FormatString(char* out, const char* format, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    i32 written = vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);

    memcpy(out, buffer, written);

}

void LogMessage(LogLevel level, const char* message, ...)
{
    const char* LogTypes[5] = {"[FATAL]: ", "[ERROR]: ", "[WARNING]: ", "[DEBUG]: ", "[TRACE]: "};

    va_list args;
    va_start(args, message);
    char buffer[LOG_BUFFER_SIZE];
    memset(buffer, 0, LOG_BUFFER_SIZE);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);

    FormatString(buffer, "%s%s\n", LogTypes[level], buffer);

    HANDLE hndl = GetStdHandle(STD_OUTPUT_HANDLE);
    static u8 levels[5] = {64, 4, 6, 2, 15};
    SetConsoleTextAttribute(hndl, levels[level]);
    u64 length = strlen(buffer);
    LPDWORD nWritten = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer, (DWORD)length, nWritten, NULL);
}

void ReportAssert(const char* expr, const char* message, const char* file, i32 line, ...)
{
    va_list args;
    va_start(args, message);
    char buffer[LOG_BUFFER_SIZE];
    memset(buffer, 0, LOG_BUFFER_SIZE);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);

    LogMessage(LOG_LEVEL_FATAL, "Assertion failed: '%s'.\n Message: '%s', in file '%s', line '%d'.", expr, buffer, file, line);
}
