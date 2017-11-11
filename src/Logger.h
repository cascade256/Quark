#pragma once
#include "tinycthread.h"
#include <stdio.h>
#include <cassert>
#include <cstring>
#include <cstdarg>
#include "Defines.h"

#define MAX_LOG_LEN 1024

enum LogLevel {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR
};

struct LoggerData {
    mtx_t logMutex;
    LogLevel logLevel;
    char* buffer;
    int bufferLen;
    int bufferIdx;
};

void initLog(LogLevel level);
void flushLog();

EXPORT void logFormat(const char* file, int line, LogLevel level, const char* formatStr, ...);
#define logD(message, ...) logFormat(__FILE__, __LINE__, LOG_DEBUG, message, ##__VA_ARGS__)
#define logI(message, ...) logFormat(__FILE__, __LINE__, LOG_INFO, message, ##__VA_ARGS__)
#define logW(message, ...) logFormat(__FILE__, __LINE__, LOG_WARNING, message, ##__VA_ARGS__)
#define logE(message, ...) logFormat(__FILE__, __LINE__, LOG_ERROR, message, ##__VA_ARGS__)
