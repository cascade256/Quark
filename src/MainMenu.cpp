#include "MainMenu.h"
#include "Globals.h"

void jobbedEvent(void* func) {
	((Func)func)();
}

void drawSubMenu(SubMenu subMenu) {
	if (nk_menu_begin_label(g->ctx, subMenu.name, NK_TEXT_LEFT, nk_vec2i(120, 200))) {
		nk_layout_row_dynamic(g->ctx, 25, 1);
		for (int i = 0; i < subMenu.items.len; i++) {
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
	nk_layout_row_begin(g->ctx, NK_STATIC, menuBarHeight, g->menus.len);
	for (int i = 0; i < g->menus.len; i++) {
		//TODO: Find a better solution that is font and font size independent
		nk_layout_row_push(g->ctx, strlen(g->menus[i].name) * 13);
		drawSubMenu(g->menus[i]);
	}
	nk_menubar_end(g->ctx);
}

int addMenu(const char* name) {
	SubMenu menu;
	menu.name = new char[strlen(name) + 1];
	strcpy(menu.name, name);
	arrayInit(&menu.items);

	arrayAdd(&g->menus, menu);
	return g->menus.len - 1;
}


void addMenuItem(int menuIdx, const char* name, Func onClick) {
	MenuItem item;
	item.name = new char[strlen(name) + 1];
	strcpy(item.name, name);
	item.onClick = onClick;

	arrayAdd(&g->menus[menuIdx].items, item);
}
