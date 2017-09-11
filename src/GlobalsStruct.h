#pragma once

//The struct is in a seperate file from the 'extern Global* g' definition so that
//plugins can load only the struct

#include "OpenFileManager.h"
#include "NuklearAndConfig.h"
#include "hashmap.h"
#include "tinycthread.h"
#include "MainMenu.h"

struct GLFWwindow;

struct Global {
	GLFWwindow* win;
	OpenFiles files;
	mtx_t filesMutex;
	int activeFileIndex;
	nk_context* ctx;
	map_t colorizers;
	map_t autocompleters;
	nk_color* theme;
	nk_color background;
	MainMenu mainMenu;
};
