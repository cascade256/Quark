#include "build.h"


//using namespace nlohmann;

#ifdef _WIN32
#include <Windows.h>
#include <sys\stat.h>
#endif



#ifdef _WIN32
#define COMPILER_CMD "\"C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/vcvarsall.bat\" amd64 && cl /MDd "
void compileFile(const char* path) {
	bool r = system("\"C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/vcvarsall.bat\" && cl /c ../src/main.cpp > test.txt ");
	printf("Test result: %i\n", r);
}
void compileTarget(TargetCompileData data) {
	int cmdLen = sizeof(COMPILER_CMD);

	cmdLen += strlen("/Fe") + strlen(data.name) + strlen(".exe ");

	for (int i = 0; i < data.sourceFiles.len; i++) {
		cmdLen += strlen(data.sourceFiles.data[i]) + 1;//+1 for the space
	}

	for (int i = 0; i < data.includeDirs.len; i++) {
		cmdLen += strlen(data.includeDirs.data[i]) + 4;//+4 for "/I <LIBRARY> "
	}

	for (int i = 0; i < data.libs.len; i++) {
		cmdLen += strlen(data.libs.data[i]) + 1;//+1 for the space
	}

	char* cmd = new char[cmdLen];
	int offset = 0;
	strcpy(cmd, COMPILER_CMD);
	offset += sizeof(COMPILER_CMD) - 1;

	strcpy(cmd + offset, "/Fe");
	offset += 3;
	strcpy(cmd + offset, data.name);
	offset += strlen(data.name);
	strcpy(cmd + offset, ".exe ");
	offset += 5;

	for (int i = 0; i < data.sourceFiles.len; i++) {
		strcpy(cmd + offset, data.sourceFiles.data[i]);
		offset += strlen(data.sourceFiles.data[i]);
		cmd[offset] = ' ';
		offset++;
	}

	for (int i = 0; i < data.includeDirs.len; i++) {
		strcpy(cmd + offset, "/I ");
		offset += 3;
		strcpy(cmd + offset, data.includeDirs.data[i]);
		offset += strlen(data.includeDirs.data[i]);
		cmd[offset] = ' ';
		offset++;
	}

	for (int i = 0; i < data.libs.len; i++) {
		strcpy(cmd + offset, data.libs.data[i]);
		offset += strlen(data.libs.data[i]);
		cmd[offset] = ' ';
		offset++;
	}

	cmd[offset] = '\0';

	printf("Command: %s\n", cmd);
	system(cmd);
	delete[] cmd;
}

time_t getFileTime(const char* path) {
	struct _stat buffer;
	_stat(path, &buffer);
	return buffer.st_mtime;
}
bool fileExists(const char* path) {
	struct _stat buffer;
	return (_stat(path, &buffer) == 0);
}
#endif

void printTime(const tm* t) {
	printf("%i-%i-%i", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
}


char* getObjFileName(const char* path) {
	const char* fileName = strrchr(path, '/') + 1;
	int fileNameLen = strlen(fileName);
	char* objName = new char[fileNameLen + 3];
	strncpy(objName, fileName, fileNameLen);
	char* ext = (char*)strrchr(objName, '.');
	strcpy(ext, ".obj");
	return objName;
}

//Check if the object file is either older than the source file or does not exist
//TODO: Make more extensive checks, opening the file and checking to see if the included files have been modified
bool isObjFileDirty(const char* path) {
	return true;
	char* objName = getObjFileName(path);
	printf("Object file name: %s\n", objName);

	if (!fileExists(objName)) {
		return true;
	}

	time_t sourceTime = getFileTime(path);
	time_t objTime = getFileTime(objName);

	delete[] objName;

	return sourceTime > objTime;
}

void buildProject(Project project) {
	for (int i = 0; i < project.targets.len; i++) {
		compileTarget(project.targets.data[i]);
	}
}