#pragma once
#ifdef _WIN32
#include <Windows.h>
#endif
#include <stdio.h>
#include "dirent.h"
#include "PluginProtocol.h"
#include "Globals.h"
#include <vector>

void loadPlugins();
void destroyPlugins();