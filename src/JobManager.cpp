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
static cnd_t jobsAvailableCND;
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
	mtx_t tssMutex;
	mtx_init(&tssMutex, mtx_plain);
	while (true) {
		JobItem* job;
		bool jobsAvailable;
		mtx_lock(&jobsListMutex);
		jobsAvailable = (jobs != NULL);
		if (!jobsAvailable) {
			mtx_unlock(&jobsListMutex);
			cnd_wait(&jobsAvailableCND, &tssMutex);
			logD("Jobs became available! (%i)\n", i);
		}
		else {
			job = jobs;
			jobs = job->next;
			mtx_unlock(&jobsListMutex);
			cnd_broadcast(&jobsAvailableCND);
			logD("Took job(%i)\n", i);
			job->func(job->data);
			delete job;
			logD("Finished job(%i)\n", i);
		}
	}
	return 0;
}

void testFunc(void* arg) {
	logD("Test: %i\n", (int)arg);
}

void initJobManager() {
	cnd_init(&jobsAvailableCND);
	mtx_init(&m, mtx_plain);
	mtx_init(&jobsListMutex, mtx_plain);

	numThreads = getNumCores() - 1;//Count the starting thread
	threads = new thrd_t[numThreads];

	logI("NumThreads: %i\n", numThreads);

	for (int i = 0; i < numThreads; i++) {
		thrd_create(&threads[i], launchFunc, (void*)i);
	}

	jobs = NULL;
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
	logD("Added Job, waking up threads(%i)\n", thrd_current());
	cnd_broadcast(&jobsAvailableCND);
}

