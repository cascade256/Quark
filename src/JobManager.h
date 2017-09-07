#pragma once
#include "tinycthread.h"
#include <stdio.h>
#include "Logger.h"

#ifdef __linux__ 
#include <unistd.h>
#endif

typedef void(*JobFunc)(void*);

struct Job {
	JobFunc func;
	void* data;
};

void initJobManager();
void addJob(JobFunc func, void* arg);
