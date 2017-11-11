#pragma once
#include "tinycthread.h"
#include <stdio.h>
#include "Logger.h"
#include "Defines.h"

#ifdef __linux__ 
#include <unistd.h>
#endif

void initJobManager();

extern "C" {
	EXPORT void addJobWithArgs(FuncWithArgs func, void* args);
	EXPORT void addJob(Func func);
}