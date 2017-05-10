#include "JobManager.h"

using namespace std;

static thrd_t* threads;
static int numThreads;

//An item in a linked list of jobs
struct JobItem {
	void* data;
	void(*func)(void*);
	JobItem* next;
};

static JobItem* jobs;
static cnd_t jobsAvailable;
static mtx_t jobsListMutex;
static mtx_t m;

int getNumCores() {
#ifdef _WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#elif __linux__
	return sysconf(_SC_NPROCESSORS_ONLN);
#else
#error
#endif
}

int launchFunc(void* i) {
	
	while (true) {
		JobItem* job;
		if (jobs == NULL) {
			cnd_wait(&jobsAvailable, &m);
			printf("Jobs became available!\n");
		}
		else {
			mtx_lock(&jobsListMutex);
			job = jobs;
			jobs = job->next;
			mtx_unlock(&jobsListMutex);
			job->func(job->data);
			delete job;
			printf("Finished job\n");
		}
	}
	return 0;
}

void testFunc(void* arg) {
	printf("Test: %i\n", (int)arg);
}

void initJobManager() {
	cnd_init(&jobsAvailable);
	mtx_init(&m, mtx_plain);
	mtx_init(&jobsListMutex, mtx_plain);

	numThreads = getNumCores() - 1;//Count the starting thread
	threads = new thrd_t[numThreads];

	printf("NumThreads: %i\n", numThreads);

	for (int i = 0; i < numThreads; i++) {
		thrd_create(&threads[i], launchFunc, (void*)i);
	}

	jobs = NULL;

	/*
	for (int i = 0; i < 10000; i++) {
		addJob(testFunc, (void*)i);
	}*/
}

void addJob(JobFunc func, void* arg) {
	JobItem* item = new JobItem();
	item->func = func;
	item->data = arg;
	item->next = NULL;

	mtx_lock(&jobsListMutex);

	if (jobs == NULL) {
		jobs = item;
	}
	else {
		JobItem* curr = jobs;
		while (curr->next != NULL) {
			curr = curr->next;
		}
		curr->next = item;
	}

	mtx_unlock(&jobsListMutex);

	cnd_signal(&jobsAvailable);
}

