#include "JobManager.h"

using namespace std;

static thrd_t* threads;
static int numThreads;

//An item in a linked list of jobs
struct Job {
	bool hasArgs;
	union {
		struct {
			FuncWithArgs funcWithArgs;
			void* args;
		};
		Func func;
	};
	Job* next;
};

static Job* jobs;
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
		Job* job;
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
			if (job->hasArgs) {
				job->funcWithArgs(job->args);
			}
			else {
				job->func();
			}
			delete job;
			logD("Finished job(%i)\n", i);
		}
	}
	return 0;
}

void testFunc(void* arg) {
	logD("Test: %i\n", arg);
}

void initJobManager() {
	cnd_init(&jobsAvailableCND);
	mtx_init(&m, mtx_plain);
	mtx_init(&jobsListMutex, mtx_plain);

	numThreads = getNumCores() - 1;//Leave one core for just the GUI
	threads = new thrd_t[numThreads];

	logI("NumThreads: %i\n", numThreads);

	for (int i = 0; i < numThreads; i++) {
		thrd_create(&threads[i], launchFunc, (void*)(intptr_t)i);
	}

	jobs = NULL;
}

void insertJobIntoList(Job* job) {
	mtx_lock(&jobsListMutex);

	if (jobs == NULL) {
		jobs = job;
	}
	else {
		Job* curr = jobs;
		while (curr->next != NULL) {
			curr = curr->next;
		}
		curr->next = job;
	}

	mtx_unlock(&jobsListMutex);
	logD("Added Job, waking up threads(%i)\n", thrd_current());
	cnd_broadcast(&jobsAvailableCND);
}

EXPORT void addJobWithArgs(FuncWithArgs func, void* args) {
	Job* job = new Job();
	job->hasArgs = true;
	job->funcWithArgs = func;
	job->args = args;
	job->next = NULL;
	insertJobIntoList(job);
}

EXPORT void addJob(Func func) {
	Job* job = new Job();
	job->hasArgs = false;
	job->func = func;
	job->next = NULL;

	insertJobIntoList(job);
}
