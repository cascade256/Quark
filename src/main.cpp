#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string>
#include "gl3w.h"

#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "NuklearAndConfig.h"
#include "nuklear/nuklear_glfw_gl3.h"


#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#include <unistd.h>
#endif

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

Global* g;

void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mods);

void sleepJob(void*) {
	logD("Start sleeping\n");
	timespec timeToSleep;
	timeToSleep.tv_sec = 10;
	thrd_sleep(&timeToSleep, 0);
	logD("Done Sleeping\n");
}

void jobsTest() {
	addJob(sleepJob, NULL);
	addJob(jobbedOpenFile, "..\\meson.build");
}

void printActiveFile() {
	TextBuffer* b = &g->files.openFiles[g->activeFileIndex].edit.buffer;
	for (int i = 0; i < b->len; i++) {
		for (int j = 0; j < b->lines[i].len; j++) {
			putchar(b->lines[i].text[j]);
		}
	}
}

int main() {
	initLog(LOG_DEBUG);
	initJobManager();

	g = new Global;
	g->colorizers = hashmap_new();
	g->autocompleters = hashmap_new();
	g->activeFileIndex = 0;
	addMenuItem(addMenu("File"), "Open", NULL);
	
	int testMenuID = addMenu("Test");
	addMenuItem(testMenuID, "Jobs Test", jobsTest);
	addMenuItem(testMenuID, "Print Active File", printActiveFile);
	

	loadPlugins();

	glfwInit();
	g->win = glfwCreateWindow(1200, 800, "Quark", NULL, NULL);
	glfwMakeContextCurrent(g->win);
	gl3wInit();

	glViewport(0, 0, 1200, 800);
//	glewExperimental = 1;
//	glewInit();

	g->ctx = nk_glfw3_init(g->win, NK_GLFW3_INSTALL_CALLBACKS, keyHandler);
	nk_font_atlas* atlas;
	nk_font* font;
	/*
	nk_font_atlas_init_default(&atlas);
	nk_font_atlas_begin(&atlas);
	font = nk_font_atlas_add_from_file(&atlas, "DroidSansMono.ttf", 16.0f, NULL);
	int w, h;
	const void* image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
	nk_glfw3_device_upload_atlas(image, w, h);
	nk_font_atlas_end(&atlas, nk_handle_id(glfw.ogl.font_tex), &glfw.ogl.null);
	nk_init_default(g->ctx, &font->handle);
	*/
	nk_glfw3_font_stash_begin(&atlas);
	font = nk_font_atlas_add_from_file(atlas, "DroidSansMono.ttf", 16, NULL);
	nk_glfw3_font_stash_end();
	nk_style_set_font(g->ctx, &font->handle);

	char* mem = new char[1024];
	int len = 1;
	strcpy(mem, "HALLO WORLD!@!!!!");
	len = strlen(mem);
	nk_text_edit edit;
	nk_textedit_init_default(&edit);
	nk_textedit_text(&edit, mem, 1024);

	FileTreeItem fileTree;
	char* path = new char[1024];
#ifdef _WIN32
	GetCurrentDirectory(1024, path);
#else
		getcwd(path, 1024);
#endif
	//printf("Path: %s\n", path);
	logD("Path: %s\n", path);
	strcat(path, "/..");
	createFileTree(path, &fileTree);
	delete[] path;

	g->theme = new nk_color[12];

	g->theme[TOK_DEFAULT] = nk_rgb(131, 148, 150);
	g->theme[TOK_COMMENT] = nk_rgb(38, 139, 210);
	g->theme[TOK_IDENTIFIER] = nk_rgb(238, 232, 213);
	g->theme[TOK_STRING] = nk_rgb(42, 161, 152);
	g->theme[TOK_NUMBER] = nk_rgb(42, 161, 152);
	g->theme[TOK_RESERVED] = nk_rgb(203, 75, 22);

	g->files.len = 0;
	g->files.openFiles = NULL;
	mtx_init(&g->filesMutex, mtx_plain);
	//openFile(&g->files, "x:\\NewProjects\\OdinEditor2\\OdinEditor\\OdinEditor\\cura_app.py");
	//openFile(&g->files, "x:\\NewProjects\\OdinEditor2\\sample.py");
	//openFile("x:\\NewProjects\\OdinEditor2\\FileTree.h");
	addJob(jobbedOpenFile, (void*)"x:\\NewProjects\\OdinEditor2\\FileTree.h");
	//openFile("~/test.h");

	g->ctx->style.edit.selected_normal.a = 200;
	g->ctx->style.edit.selected_normal.b = 200;

	//glfwWindowShouldClose can be set from within nuklear_glfw_gl3.h keypress handler
	while (!glfwWindowShouldClose(g->win)) {
		flushLog();
		glfwPollEvents();
		nk_glfw3_new_frame();
		int width;
		int height;
		glfwGetWindowSize(g->win, &width, &height);


		if (nk_begin(g->ctx, "Demo", nk_rect(0, 0, width, height),
			NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {

			int windowHeaderHeight = 30;
			//Menu bar
			/*
			int menuBarHeight = 25;
			nk_menubar_begin(g->ctx);
			nk_layout_row_begin(g->ctx, NK_STATIC, menuBarHeight, 4);
			nk_layout_row_push(g->ctx, 45);
			if (nk_menu_begin_label(g->ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 200))) {
				nk_layout_row_dynamic(g->ctx, 25, 1);
				if (nk_menu_item_label(g->ctx, "Quit", NK_TEXT_LEFT)) {
					glfwSetWindowShouldClose(g->win, true);
				}
				nk_menu_end(g->ctx);
			}
			nk_menubar_end(g->ctx);
			*/
			drawMainMenu();

			struct nk_rect region = nk_window_get_content_region(g->ctx);
			float ratio = 0.2f;
			nk_label(g->ctx, "Default", NK_TEXT_LEFT);

			static float fileTreeWidth = 400;
			static float textEditorWidth = region.w - 380;

			textEditorWidth = region.w - fileTreeWidth - 50;
			float row_layout[3];
			row_layout[0] = fileTreeWidth;
			row_layout[1] = 8;
			row_layout[2] = textEditorWidth;
			nk_layout_row(g->ctx, NK_STATIC, region.h - 10, 3, row_layout);

			//File explorer
			if (nk_group_begin(g->ctx, "Files", NK_WINDOW_BORDER))
			{
				drawFileTree(g->ctx, &fileTree, &g->files);
				nk_group_end(g->ctx);
			}

			struct nk_rect bounds = nk_widget_bounds(g->ctx);
			nk_spacing(g->ctx, 1);
			if ((nk_input_is_mouse_hovering_rect(&g->ctx->input, bounds) ||
				nk_input_is_mouse_prev_hovering_rect(&g->ctx->input, bounds)) &&
				nk_input_is_mouse_down(&g->ctx->input, NK_BUTTON_LEFT))
			{
				fileTreeWidth = row_layout[0] + g->ctx->input.mouse.delta.x;
				if (fileTreeWidth <= 100) {
					fileTreeWidth = 100;
				}
			}


			drawTextEditor(g->ctx, &g->files);
		}
		nk_end(g->ctx);


		//overview(g->ctx);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.5f, 0.5f, 0.5f, 1);
		nk_glfw3_render(NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
		glfwSwapBuffers(g->win);
	}

	delete[] mem;
	delete[] g->theme;
	destroyFileTree(&fileTree);
	destroyFiles(&g->files);
	destroyPlugins();
	nk_glfw3_shutdown();
	glfwTerminate();
//	_CrtDumpMemoryLeaks();
	return 0;
}


void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mods) {
	if ((mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL) {
		switch (key) {
		case GLFW_KEY_S:
			saveActiveFile(&g->files);
			break;
		case GLFW_KEY_F:
			showSearchDialog();
			break;
		}
	}
}
