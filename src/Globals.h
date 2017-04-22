#pragma once
#include "OpenFileManager.h"
#include "NuklearAndConfig.h"
#include "nuklear/nuklear_glfw_gl3.h"
#include "hashmap.h"

struct Global {
	GLFWwindow* win;
	OpenFiles files;
	nk_context* ctx;
	map_t colorizers;
	map_t autocompleters;
	nk_color* theme;
	nk_color background;
};

extern Global* g;
