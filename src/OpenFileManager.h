#pragma once
#include <stdio.h>
#include <string>
#include <cassert>
#include "NuklearAndConfig.h"
#include "PluginProtocol.h"

struct MyOpenFile {
	char* path;
	char* name;
	int nameLen;//Does not include the NULL byte, but does include the possible *
	bool unsaved = false;
	Colorize_Func colorize = NULL;
	struct nk_my_text_edit edit;
};

struct OpenFiles {
	MyOpenFile* openFiles;
	int len;
};


void jobbedOpenFile(void* path);
void openFile(const char* path);
void saveFile(MyOpenFile* file);
void destroyFiles(OpenFiles* files);

