#pragma once
#include "PluginProtocol.h"
#include "NuklearAndConfig.h"
	
struct MenuItem {
	char* name;
	Func onClick;
};

struct SubMenu{
	char* name;
	MenuItem* items;
	int numItems;
};


struct MainMenu {
	SubMenu* subMenus = NULL;
	int numSubMenus = 0;
};

void drawMainMenu();
void addMenuItem(int menuIDX, const char* item, Func onClick);
int addMenu(const char* name);