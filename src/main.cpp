#ifdef _WIN32
#include <Windows.h>
#include "dirent.h"
#elif __linux__
#include <dirent.h>
#include <unistd.h>
#include <dlfcn.h>
#endif
#include "GlobalsStruct.h"

bool reload;
bool(*initQuark)(Global*) = NULL;
bool(*updateQuark)(Global*) = NULL;
bool(*shutdownQuark)(Global*) = NULL;

#ifdef _WIN32 
HINSTANCE dll;
#elif __linux__
void* dll;
#endif

bool loadQuarkCore() {
	//Load the DLL and setup the function pointers
	char* libPath = new char[1024];
	char* copyPath = new char[1024];

#ifdef _WIN32
	GetModuleFileName(NULL, libPath, 1024);
	char* fileName = strrchr(libPath, '\\');
	strncpy(fileName, "\\QuarkCore.dll", 1024 - (fileName - libPath));

	strcpy(copyPath, libPath);
	strcat(copyPath, ".loaded");

	CopyFile(libPath, copyPath, false);

	dll = LoadLibrary(copyPath);
	if (!dll) {
		printf("Failed to load QuarkCore!\n");
		return false;
	}

	initQuark = (bool(*)(Global*))GetProcAddress(dll, "initQuark");
	updateQuark = (bool(*)(Global*))GetProcAddress(dll, "updateQuark");
	shutdownQuark = (bool(*)(Global*))GetProcAddress(dll, "shutdownQuark");
#elif __linux__
	int len = readlink("/proc/self/exe", libPath, 1024);
	libPath[len] = '\0';
	char* fileName = strrchr(libPath, '/');
	strncpy(fileName, "/libQuarkCore.so", 1024 - (fileName - libPath));

	dll = dlopen(libPath, RTLD_NOW);
	if (!dll) {
		printf("Failed to load QuarkCore!: %s\n", dlerror());
		return false;
	}

	initQuark = (bool(*)(Global*))dlsym(dll, "initQuark");
	updateQuark = (bool(*)(Global*))dlsym(dll, "updateQuark");
	shutdownQuark = (bool(*)(Global*))dlsym(dll, "shutdownQuark");
#else
#error "Unsupported platform!"
#endif
	assert(initQuark);
	assert(updateQuark);
	assert(shutdownQuark);

	delete[] libPath;
	delete[] copyPath;
	return true;
}

int main() {
	loadQuarkCore();
	Global* g = new Global();
	initQuark(g);
	while(updateQuark(g)) {
		if (reload) {
#ifdef _WIN32
			FreeLibrary(dll);
#elif __linux__
			dlclose(dll);
#endif
			loadQuarkCore();
			reload = false;
		}
	}
	shutdownQuark(g);

	return 0;
}
