#pragma once

#include "NuklearAndConfig.h"
#include <cassert>
#include "OpenFileManager.h"
#include "JobManager.h"

struct FileTreeItem {
	int type;//dirent::d_type
	char* name;
	char* path;
	FileTreeItem* subItems;
	int subItemCount;
	int selected;
	bool loaded;
};

void createFileTree(const char* path, FileTreeItem* fileTree);
void extendFileTree(FileTreeItem* fileTree);
void destroyFileTree(FileTreeItem* fileTree);
void drawFileTree(nk_context* ctx, FileTreeItem* fileTree, OpenFiles* files);