#pragma once

#include "NuklearAndConfig.h"
#include <cassert>
#include "OpenFileManager.h"
#include "JobManager.h"
#include "Array.h"

struct FileTreeFile {
	char* name;
	char* path;
	int selected;
};

struct FileTreeFolder {
	char* name;
	char* path;
	Array<FileTreeFolder> folders;
	Array<FileTreeFile> files;
	int selected;
	bool loaded;
};

void createFileTree(const char* path, FileTreeFolder* fileTree);
void extendFileTree(FileTreeFolder* fileTree);
void destroyFileTree(FileTreeFolder* fileTree);
void drawFileTree(nk_context* ctx, FileTreeFolder* fileTree, OpenFiles* files);
