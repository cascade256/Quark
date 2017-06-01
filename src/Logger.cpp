#include "Logger.h"

const char* logLevelStrs[] = { "DEBUG", "INFO", "WARNING", "ERROR" };

static mtx_t logMutex;
static LogLevel logLevel;


#define MAX_LOG_LEN 1024

static char* buffer;
static int bufferLen;
static int bufferIdx;


void initLog(LogLevel level) {
	bufferLen = MAX_LOG_LEN * 30;
	buffer = new char[bufferLen];
	bufferIdx = 0;

	mtx_init(&logMutex, mtx_plain);
	logLevel = level;
}

void logFormat(const char* file, int line, LogLevel level, const char* formatStr, ...) {
	if (level < logLevel) {
		return;
	}
	
	mtx_lock(&logMutex);
	//Check to make sure that there is room for the max message length
	if (bufferLen - bufferIdx < MAX_LOG_LEN) {
		bufferLen *= 2;
		char* old = buffer;
		buffer = new char[bufferLen];
		memcpy(buffer, old, bufferIdx);
		delete[] old;
	}

	int numWritten = sprintf(&buffer[bufferIdx], "%s:[%s:%i] ", logLevelStrs[level], file, line);

	va_list args;
	va_start(args, formatStr);
	numWritten += vsprintf(&buffer[bufferIdx + numWritten], formatStr, args);
	va_end(args);

	assert(numWritten <= MAX_LOG_LEN);
	bufferIdx += numWritten;
	mtx_unlock(&logMutex);
}

void flushLog() {
	if (bufferIdx == 0) {
		return;
	}
	mtx_lock(&logMutex);
	puts(buffer);
	bufferIdx = 0;
	buffer[0] = '\0';
	mtx_unlock(&logMutex);
}