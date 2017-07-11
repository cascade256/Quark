#pragma once
#include "../Array.h"

enum TargetType {
	TARGET_TYPE_EXECUTABLE,
	TARGET_TYPE_STATIC_LIB,
	TARGET_TYPE_SHARED_LIB
};

struct TargetCompileData {
	char* name;
	TargetType targetType;
	Array<char*> sourceFiles;
	Array<char*> includeDirs;
	Array<char*> libs;
};


struct Project {
	Array<TargetCompileData> targets;
};
