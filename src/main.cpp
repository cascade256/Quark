#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <string>
#include "gl3w.h"

#include "NuklearAndConfig.h"
#include "nuklear/nuklear_glfw_gl3.h"

#include <string.h>
#include "FileTree.h"
#include "OpenFileManager.h"
#include "TextEditor.h"
#include "PluginManager.h"
#include "hashmap.h"
#include "Globals.h"
#include "JobManager.h"
#include "MainMenu.h"
#include "Logger.h"
#include "LayoutManager.h"
#include "Array.h"

Global* g;
FileTreeFolder fileTree;

void controlKeyHandler(int key, int action);

void drawFileTree() {

	nk_layout_row_dynamic(g->ctx, nk_window_get_content_region(g->ctx).h, 1);
	if (nk_group_begin(g->ctx, "Files", NK_WINDOW_BORDER))
	{
		drawFileTree(g->ctx, &fileTree, &g->files);
		nk_group_end(g->ctx);
	}
}

void drawTextEditor() {
	nk_layout_row_dynamic(g->ctx, nk_window_get_content_region(g->ctx).h, 1);
	drawTextEditor(g->ctx, &g->files);
}

int main() {
	initLog(LOG_DEBUG);
	initJobManager();

	g = new Global;
	g->colorizers = hashmap_new();
	g->autocompleters = hashmap_new();
	g->activeFileIndex = 0;

	arrayInit(&g->menus);
	addMenuItem(addMenu("File"), "Open", NULL);

	loadPlugins();

	glfwInit();
	g->win = glfwCreateWindow(1200, 800, "Quark", NULL, NULL);
	glfwMakeContextCurrent(g->win);
	gl3wInit();

	glViewport(0, 0, 1200, 800);

	g->ctx = nk_glfw3_init(g->win, NK_GLFW3_INSTALL_CALLBACKS, controlKeyHandler);

	setupTheme();

	char* path = new char[1024];
#ifdef _WIN32
	GetCurrentDirectory(1024, path);
#else
	getcwd(path, 1024);
#endif

	logD("Path: %s\n", path);
	strcat(path, "/..");;
    createFileTree(path, &fileTree);
	delete[] path;

	g->files.len = 0;
	g->files.openFiles = NULL;
	mtx_init(&g->filesMutex, mtx_plain);

	g->ctx->style.edit.selected_normal.a = 200;
	g->ctx->style.edit.selected_normal.b = 200;

	View* view = createSplitViewPixel(createView(drawFileTree, "File Tree"), createView(drawTextEditor, "Text Editor"), false, true, 300);

	//glfwWindowShouldClose can be set from within nuklear_glfw_gl3.h keypress handler
	while (!glfwWindowShouldClose(g->win)) {
		flushLog();
		glfwPollEvents();
		nk_glfw3_new_frame();
		int width;
		int height;
		glfwGetWindowSize(g->win, &width, &height);

		nk_begin(g->ctx, "Main", nk_rect(0, 0, width, height), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR);
		drawMainMenu();
		drawLayout(view, width, height);
		nk_end(g->ctx);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.5f, 0.5f, 0.5f, 1);
		nk_glfw3_render(NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
		glfwSwapBuffers(g->win);
	}

	destroyTheme();
	destroyFileTree(&fileTree);
	destroyFiles(&g->files);
	destroyPlugins();
	nk_glfw3_shutdown();
	glfwTerminate();
//	_CrtDumpMemoryLeaks();
	return 0;
}


void controlKeyHandler(int key, int action) {
	switch (key) {
	case GLFW_KEY_S:
		saveActiveFile(&g->files);
		break;
	case GLFW_KEY_F:
		showSearchDialog();
		break;
	}	
}

