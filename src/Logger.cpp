#include "Logger.h"
#include "Globals.h"

const char* logLevelStrs[] = { "DEBUG", "INFO", "WARNING", "ERROR" };

void initLog(LogLevel level) {
	g->logData.bufferLen = MAX_LOG_LEN * 30;
	g->logData.buffer = new char[g->logData.bufferLen];
	g->logData.bufferIdx = 0;

	mtx_init(&g->logData.logMutex, mtx_plain);
	g->logData.logLevel = level;
}

EXPORT void logFormat(const char* file, int line, LogLevel level, const char* formatStr, ...) {
	if (level < g->logData.logLevel) {
		return;
	}

	mtx_lock(&g->logData.logMutex);
	//Check to make sure that there is room for the max message length
	if (g->logData.bufferLen - g->logData.bufferIdx < MAX_LOG_LEN) {
		g->logData.bufferLen *= 2;
		char* old = g->logData.buffer;
		g->logData.buffer = new char[g->logData.bufferLen];
		memcpy(g->logData.buffer, old, g->logData.bufferIdx);
		delete[] old;
	}

	int numWritten = sprintf(&g->logData.buffer[g->logData.bufferIdx], "%s:[%s:%i] ", logLevelStrs[level], file, line);

	va_list args;
	va_start(args, formatStr);
	numWritten += vsprintf(&g->logData.buffer[g->logData.bufferIdx + numWritten], formatStr, args);
	va_end(args);

	assert(numWritten <= MAX_LOG_LEN);
	g->logData.bufferIdx += numWritten;
	mtx_unlock(&g->logData.logMutex);
}

void flushLog() {
	if (g->logData.bufferIdx == 0) {
		return;
	}
	mtx_lock(&g->logData.logMutex);
	puts(g->logData.buffer);
	g->logData.bufferIdx = 0;
	g->logData.buffer[0] = '\0';
	mtx_unlock(&g->logData.logMutex);
}
