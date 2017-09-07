#include "MainMenu.h"
#include "Globals.h"

void jobbedEvent(void* func) {
	((Func)func)();
}

void drawSubMenu(SubMenu subMenu) {
	if (nk_menu_begin_label(g->ctx, subMenu.name, NK_TEXT_LEFT, nk_vec2i(120, 200))) {
		nk_layout_row_dynamic(g->ctx, 25, 1);
		for (int i = 0; i < subMenu.numItems; i++) {
			if (nk_menu_item_label(g->ctx, subMenu.items[i].name, NK_TEXT_LEFT)) {
				if (subMenu.items[i].onClick) {
					addJob(jobbedEvent, (void*)subMenu.items[i].onClick);
				}
			}
		}
		nk_menu_end(g->ctx);
	}
}

void drawMainMenu() {
	int menuBarHeight = 25;
	nk_menubar_begin(g->ctx);
	nk_layout_row_begin(g->ctx, NK_STATIC, menuBarHeight, 4);
	nk_layout_row_push(g->ctx, 45);
	for (int i = 0; i < g->mainMenu.numSubMenus; i++) {
		drawSubMenu(g->mainMenu.subMenus[i]);
	}
	nk_menubar_end(g->ctx);
}

int addMenu(const char* name) {
	if (g->mainMenu.subMenus) {
		SubMenu* oldMenus = g->mainMenu.subMenus;
		g->mainMenu.subMenus = new SubMenu[g->mainMenu.numSubMenus + 1];
		memcpy(g->mainMenu.subMenus, oldMenus, g->mainMenu.numSubMenus * sizeof(SubMenu));
		g->mainMenu.subMenus[g->mainMenu.numSubMenus].items = NULL;
		g->mainMenu.subMenus[g->mainMenu.numSubMenus].numItems = 0;
		g->mainMenu.subMenus[g->mainMenu.numSubMenus].name = new char[strlen(name)];
		strcpy(g->mainMenu.subMenus[g->mainMenu.numSubMenus].name, name);
		g->mainMenu.numSubMenus++;
		return g->mainMenu.numSubMenus - 1;
	}
	else {
		assert(g->mainMenu.numSubMenus == 0);
		g->mainMenu.subMenus = new SubMenu[g->mainMenu.numSubMenus + 1];
		g->mainMenu.subMenus[g->mainMenu.numSubMenus].items = NULL;
		g->mainMenu.subMenus[g->mainMenu.numSubMenus].numItems = 0;
		g->mainMenu.subMenus[g->mainMenu.numSubMenus].name = new char[strlen(name)];
		strcpy(g->mainMenu.subMenus[g->mainMenu.numSubMenus].name, name);
		g->mainMenu.numSubMenus++;
		return g->mainMenu.numSubMenus - 1;
	}
}


void addMenuItem(int menuIdx, const char* item, Func onClick) {
	assert(menuIdx < g->mainMenu.numSubMenus);
	SubMenu* menu = &g->mainMenu.subMenus[menuIdx];
	if (menu->items) {
		MenuItem* oldItems = menu->items;
		menu->items = new MenuItem[menu->numItems + 1];
		memcpy(menu->items, oldItems, menu->numItems * sizeof(MenuItem));
		menu->items[menu->numItems].name = new char[strlen(item)];
		strcpy(menu->items[menu->numItems].name, item);
		menu->items[menu->numItems].onClick = onClick;
		menu->numItems++;
	}
	else {
		assert(menu->numItems == 0);
		menu->items = new MenuItem[menu->numItems + 1];
		menu->items[menu->numItems].name = new char[strlen(item)];
		strcpy(menu->items[menu->numItems].name, item);
		menu->items[menu->numItems].onClick = onClick;
		menu->numItems++;
	}

}
