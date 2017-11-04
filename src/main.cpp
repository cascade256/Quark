#ifdef _WIN32
#include <Windows.h>
#include "dirent.h"
#elif __linux__
#include <dirent.h>
#include <unistd.h>
#include <dlfcn.h>
#endif
#include "GlobalsStruct.h"

int main() {

	bool(*initQuark)(Global*);
	bool(*updateQuark)(Global*);
	bool(*shutdownQuark)(Global*);

//Load the DLL and setup the function pointers
char* libPath = new char[1024];

#ifdef _WIN32
	GetModuleFileName(NULL, libPath, 1024);
	char* fileName = strrchr(libPath, '\\');
	strncpy(fileName, "\\libQuarkCore.dll",  1024 - (fileName - libPath));
#elif __linux__
	int len = readlink("/proc/self/exe", libPath, 1024);
	libPath[len] = '\0';
	char* fileName = strrchr(libPath, '/');
	strncpy(fileName, "/libQuarkCore.so", 1024 - (fileName - libPath));

	void* dll = dlopen(libPath, RTLD_NOW);
	if (!dll) {
		printf("Failed to load QuarkCore!: %s\n", dlerror());
		return 1;
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

	Global* g = new Global();
	initQuark(g);
	while(updateQuark(g)) {}
	shutdownQuark(g);

	return 0;
}
