#pragma once
#include "tinycthread.h"
#include <stdio.h>

typedef void(*JobFunc)(void*);

struct Job {
	JobFunc func;
	void* data;
};

void initJobManager();
void addJob(JobFunc func, void* arg);