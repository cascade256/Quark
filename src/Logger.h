#pragma once
#include "tinycthread.h"
#include <stdio.h>
#include <cassert>
#include "PluginProtocol.h"
#include <cstring>
#include <cstdarg>

void initLog(LogLevel level);
void flushLog();

void logFormat(const char* file, int line, LogLevel level, const char* formatStr, ...);
#define logD(message, ...) logFormat(__FILE__, __LINE__, LOG_DEBUG, message, ##__VA_ARGS__)
#define logI(message, ...) logFormat(__FILE__, __LINE__, LOG_INFO, message, ##__VA_ARGS__)
#define logW(message, ...) logFormat(__FILE__, __LINE__, LOG_WARNING, message, ##__VA_ARGS__)
#define logE(message, ...) logFormat(__FILE__, __LINE__, LOG_ERROR, message, ##__VA_ARGS__)
