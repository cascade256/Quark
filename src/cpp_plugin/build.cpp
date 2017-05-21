#include "build.h"


//using namespace nlohmann;

#ifdef _WIN32
#include <Windows.h>
#include <sys\stat.h>
#endif


struct TargetCompileData {
	char** sourceFiles;
	int numSourceFiles;

	char** objFiles;
	int numObjFiles;

	char** includeDirs;
	int numIncludeDirs;
	
	char** libs;
	int numLibs;
};


#ifdef _WIN32
#define COMPILER_CMD "\"C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/vcvarsall.bat\" amd64 && cl /MDd "
void compileFile(const char* path) {
	bool r = system("\"C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/vcvarsall.bat\" && cl /c ../src/main.cpp > test.txt ");
	printf("Test result: %i\n", r);
}
void compileTarget(TargetCompileData data) {
	
	int cmdLen = sizeof(COMPILER_CMD);

	for (int i = 0; i < data.numSourceFiles; i++) {
		cmdLen += strlen(data.sourceFiles[i]) + 1;//+1 for the space
	}

	for (int i = 0; i < data.numObjFiles; i++) {
		cmdLen += strlen(data.objFiles[i]) + 1;//+1 for the space
	}

	for (int i = 0; i < data.numIncludeDirs; i++) {
		cmdLen += strlen(data.includeDirs[i]) + 4;//+4 for "/I <LIBRARY> "
	}

	for (int i = 0; i < data.numLibs; i++) {
		cmdLen += strlen(data.libs[i]) + 1;//+1 for the space
	}

	char* cmd = new char[cmdLen];
	int offset = 0;
	strcpy(cmd, COMPILER_CMD);
	offset += sizeof(COMPILER_CMD) - 1;

	for (int i = 0; i < data.numSourceFiles; i++) {
		strcpy(cmd + offset, data.sourceFiles[i]);
		offset += strlen(data.sourceFiles[i]);
		cmd[offset] = ' ';
		offset++;
	}

	for (int i = 0; i < data.numObjFiles; i++) {
		strcpy(cmd + offset, data.objFiles[i]);
		offset += strlen(data.objFiles[i]);
		cmd[offset] = ' ';
		offset++;
	}

	for (int i = 0; i < data.numIncludeDirs; i++) {
		strcpy(cmd + offset, "/I ");
		offset += 3;
		strcpy(cmd + offset, data.includeDirs[i]);
		offset += strlen(data.includeDirs[i]);
		cmd[offset] = ' ';
		offset++;
	}

	for (int i = 0; i < data.numLibs; i++) {
		strcpy(cmd + offset, data.libs[i]);
		offset += strlen(data.libs[i]);
		cmd[offset] = ' ';
		offset++;
	}

	cmd[offset] = '\0';

	printf("Command: %s\n", cmd);
	system(cmd);
	delete[] cmd;
}
/*
time_t getFileTime(const char* path) {
	HANDLE file = CreateFile(path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		printf("The file %s could not be opened!\n", path);
		return 0;
	}
	FILETIME writeTime;
	if (!GetFileTime(file, NULL, NULL, &writeTime)) {
		printf("Could not get the last write time of the file: %s\n", path);
	}
	CloseHandle(file);

	SYSTEMTIME time;
	FileTimeToSystemTime(&writeTime, &time);
	return (time_t)writeTime.dwLowDateTime;
}*/

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

void buildTarget(JSON_Object* target) {
	TargetCompileData data;
	const char* output = json_object_get_string(target, "output");

	//Source and Object Files
	JSON_Array* sourceFiles = json_object_get_array(target, "sources");
	data.sourceFiles = new char*[json_array_get_count(sourceFiles)];
	data.numSourceFiles = 0;
	data.objFiles = new char*[json_array_get_count(sourceFiles)];
	data.numObjFiles = 0;

	for (int i = 0; i < json_array_get_count(sourceFiles); i++) {
		const char* sourceFile = json_array_get_string(sourceFiles, i);
		time_t t = getFileTime(sourceFile);
		tm* tt = localtime(&t);
		printTime(tt);
		printf("\n");
		
		//Check to see if we should compile the source file, or just link with the obj file
		if (isObjFileDirty(sourceFile)) {
			data.sourceFiles[data.numSourceFiles] = (char*)sourceFile;
			data.numSourceFiles++;
		}
		else {
			data.objFiles[data.numObjFiles] = getObjFileName(sourceFile);
			data.numObjFiles++;
		}
	}

	for (int i = 0; i < data.numSourceFiles; i++) {
		printf("%s\n", data.sourceFiles[i]);
	}

	//Include Dirs
	JSON_Array* includeDirsJSON = json_object_get_array(target, "include");
	data.includeDirs = new char*[json_array_get_count(includeDirsJSON)];
	data.numIncludeDirs = 0;

	for (int i = 0; i < json_array_get_count(includeDirsJSON); i++) {
		data.includeDirs[data.numIncludeDirs] = (char*)json_array_get_string(includeDirsJSON, i);
		data.numIncludeDirs++;
	}

	//Libs
	JSON_Array* libsJSON = json_object_get_array(target, "libs");
	data.libs = new char*[json_array_get_count(libsJSON)];
	data.numLibs = 0;

	for (int i = 0; i < json_array_get_count(libsJSON); i++) {
		data.libs[data.numLibs] = (char*)json_array_get_string(libsJSON, i);
		data.numLibs++;
	}

	compileTarget(data);
}

void buildProject() {

	char* buffer;

	JSON_Value* root;
	root = json_parse_file("build.json");
	JSON_Value_Type type = json_value_get_type(root);
	JSON_Object* rootObj = json_value_get_object(root);
	JSON_Array* targets = json_object_get_array(rootObj, "targets");

	for (int i = 0; i < json_array_get_count(targets); i++) {
		JSON_Value* target = json_array_get_value(targets, i);
		//JSON_Object* target = json_array_get_object(targets, i);
		if (json_value_get_type(target) == JSONObject) {
			buildTarget(json_value_get_object(target));
		}
		else {
			printf("Invalid build target!\n");
		}
	}

	printf("Number of Targets: %i\n", json_array_get_count(targets));
}