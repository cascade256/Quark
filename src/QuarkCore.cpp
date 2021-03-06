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
#include "Defines.h"

Global* g;

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

void drawFileTree() {

	nk_layout_row_dynamic(g->ctx, nk_window_get_content_region(g->ctx).h, 1);
	if (nk_group_begin(g->ctx, "Files", NK_WINDOW_BORDER))
	{
		drawFileTree(g->ctx, &g->fileTree, &g->files);
		nk_group_end(g->ctx);
	}
}

void drawTextEditor() {
	nk_layout_row_dynamic(g->ctx, nk_window_get_content_region(g->ctx).h, 1);
	drawTextEditor(g->ctx, &g->files);
}

EXPORT bool menuItemBegin(const char* text, float minWidth) {
	float width = g->ctx->style.font->width(g->ctx->style.font->userdata, g->ctx->style.font->height, text, strlen(text));
	width += 15;
	if (width < minWidth) {
		width = minWidth;
	}
	nk_layout_row_push(g->ctx, width);
	return nk_menu_begin_label(g->ctx, text, NK_TEXT_LEFT, nk_vec2i(300, g->ctx->current->bounds.h));
}

void glfwErrorCallback(int error, const char* desc) {
	logE("GLFW Error: %i, %s\n", error, desc);
}


void init() {
	g = new Global();

	initLog(LOG_DEBUG);
	initJobManager();

	g->colorizers = hashmap_new();
	g->autocompleters = hashmap_new();
	g->activeFileIndex = 0;
	g->findDialogOpen = false;

	arrayInit(&g->menubarCBs);

	loadPlugins();

	glfwSetErrorCallback(glfwErrorCallback);
	int result = glfwInit();
	if (result == false) {
		printf("Could not initialize GLFW!\n");
		exit(1);
	}

	g->win = glfwCreateWindow(1200, 800, "Quark", NULL, NULL);
	if (g->win == NULL) {
		printf("Could not create a window!\n");
		glfwTerminate();
		exit(1);
	}
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
	createFileTree(path, &g->fileTree);
	delete[] path;

	g->files.len = 0;
	g->files.openFiles = NULL;
	mtx_init(&g->filesMutex, mtx_plain);

	g->ctx->style.edit.selected_normal.a = 200;
	g->ctx->style.edit.selected_normal.b = 200;

	g->view = createSplitViewPixel(createView(drawFileTree, "File Tree"), createView(drawTextEditor, "Text Editor"), false, true, 300);
}

bool update() {
	flushLog();
	glfwPollEvents();
	nk_glfw3_new_frame();
	int width;
	int height;
	glfwGetWindowSize(g->win, &width, &height);

	nk_begin(g->ctx, "Main", nk_rect(0, 0, width, height), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR);

	//Draw main menu
	{
		int menuBarHeight = g->ctx->style.font->height + g->ctx->style.menu_button.padding.y;
		nk_menubar_begin(g->ctx);
		nk_layout_row_begin(g->ctx, NK_STATIC, menuBarHeight, g->menubarCBs.len);
		for (int i = 0; i < g->menubarCBs.len; i++) {
			g->menubarCBs[i]();
		}
		nk_layout_row_end(g->ctx);
		nk_menubar_end(g->ctx);
	}
	drawLayout(g->view, width, height);
	nk_end(g->ctx);

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 1);
	nk_glfw3_render(NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
	glfwSwapBuffers(g->win);

	GLenum error = glGetError();
	while (error != 0) {
		logW("OpenGL error: %x\n", error);
		error = glGetError();
		
	}

	return !glfwWindowShouldClose(g->win);
}

void shutdown() {
	destroyTheme();
	destroyFileTree(&g->fileTree);
	destroyFiles(&g->files);
	destroyPlugins();
	nk_glfw3_shutdown();
	glfwTerminate();
}

EXPORT void runQuark() {
	init();
	while (update()) {}
	shutdown();
}