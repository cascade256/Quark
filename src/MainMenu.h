#pragma once
#include "PluginProtocol.h"
#include "NuklearAndConfig.h"
#include "JobManager.h"
	
struct MenuItem {
	char* name;
	Func onClick;
};

struct SubMenu{
	char* name;
	Array<MenuItem> items;
};

void drawMainMenu();
void addMenuItem(int menuIDX, const char* name, Func onClick);
int addMenu(const char* name);