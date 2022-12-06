#pragma once

#define LOG_WARN_ENABLED true
#define LOG_ERROR_ENABLED true
#define LOG_ERROR_ENABLED true

#ifdef NDEBUG
    #define LOG_DEBUG_ENABLED false
    #define LOG_DEBUG_TRACE false
#else
    #define LOG_DEBUG_ENABLED true
    #define LOG_DEBUG_TRACE true
#endif

enum LogLevel 
{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_DEBUG = 3,
    LOG_LEVEL_TRACE = 4
};

void LogMessage(LogLevel level, const char* message, ...);

//void FormatString(const char* out, const char* format, ...);

#define SFATAL(message, ...) LogMessage(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);
#define SERROR(message, ...) LogMessage(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#define SWARNING(message, ...) LogMessage(LOG_LEVEL_WARNING, message, ##__VA_ARGS__);
#define SDEBUG(message, ...) LogMessage(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#define STRACE(message, ...) LogMessage(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
