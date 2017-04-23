#pragma once
#ifdef _WIN32
#include <Windows.h>
#include "dirent.h"
#else
#include <dirent.h>
#include <unistd.h>
#include <dlfcn.h>
#endif
#include <stdio.h>
#include "PluginProtocol.h"
#include "Globals.h"
#include <vector>

void loadPlugins();
void destroyPlugins();
