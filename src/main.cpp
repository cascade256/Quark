#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string>
#include "gl3w.h"

//#define NK_IMPLEMENTATION
//#define NK_GLFW_GL3_IMPLEMENTATION
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
#include "LayoutManager.h"
#include "Array.h"


Global* g;
FileTreeItem fileTree;

void controlKeyHandler(int key, int action);

void sleepJob(void*) {
	logD("Start sleeping\n");
	timespec timeToSleep;
	timeToSleep.tv_sec = 10;
	thrd_sleep(&timeToSleep, 0);
	logD("Done Sleeping\n");
}

void jobsTest() {
	addJob(sleepJob, NULL);
	addJob(jobbedOpenFile, (void*)"..\\meson.build");
}

void printActiveFile() {
	Array<TextLine> b = g->files.openFiles[g->activeFileIndex].edit.lines;
	for (int i = 0; i < b.len; i++) {
		for (int j = 0; j < b[i].text.len; j++) {
			putchar(b[i].text[j]);
		}
	}
}

void arrayTest() {
	Array<int> nums;
	arrayInit(&nums);
	arrayAdd(&nums, 1);
	arrayAdd(&nums, 2);
	arrayAdd(&nums, 3);

	for (int i = 0; i < nums.len; i++) {
		logD("ArrayTest: nums[%i] = %i \n", i, nums.data[i]);
	}

	arrayRemoveAt(&nums, 1);

	for (int i = 0; i < nums.len; i++) {
		logD("ArrayTest: nums[%i] = %i \n", i, nums.data[i]);
	}
}


void drawFileTree() {

	nk_layout_row_dynamic(g->ctx, nk_window_get_content_region(g->ctx).h, 1);
	//nk_button_text(g->ctx, "File tree button!", 17);
	if (nk_group_begin(g->ctx, "Files", NK_WINDOW_BORDER))
	{
		drawFileTree(g->ctx, &fileTree, &g->files);
		nk_group_end(g->ctx);
	}
}

void drawTextEditor() {
	nk_layout_row_dynamic(g->ctx, nk_window_get_content_region(g->ctx).h, 1);
	//nk_button_text(g->ctx, "Text editor button!", 19);
	drawTextEditor(g->ctx, &g->files);
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

	g->ctx = nk_glfw3_init(g->win, NK_GLFW3_INSTALL_CALLBACKS, controlKeyHandler);
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

	char fontFile[1024];//The directory the program is in
#ifdef _WIN32
	GetModuleFileName(NULL, fontFile, 1024);
	char* fileName = strrchr(fontFile, '\\');
	strncpy(fileName, "\\Code New Roman.ttf", 1024 - (fileName - fontFile));
#elif __linux__
	{
	    int len = readlink("/proc/self/exe", fontFile, 1024);
	    fontFile[len] = '\0';
       	char* fileName = strrchr(fontFile, '/');
	    strncpy(fileName, "/DroidSansMono.ttf", 1024 - (fileName - fontFile));
	}
#else
#error "Unsupported platform!"
#endif
	logI("Font File: %s\n", fontFile);
    printf("Font File: %s\n", fontFile);

	nk_glfw3_font_stash_begin(&atlas);
	font = nk_font_atlas_add_from_file(atlas, fontFile, 14, NULL);
	nk_glfw3_font_stash_end();
	nk_style_set_font(g->ctx, &font->handle);

	char* mem = new char[1024];
	int len = 1;
	strcpy(mem, "HALLO WORLD!@!!!!");
	len = strlen(mem);
	nk_text_edit edit;
	nk_textedit_init_default(&edit);
	nk_textedit_text(&edit, mem, 1024);


	char* path = new char[1024];
#ifdef _WIN32
	GetCurrentDirectory(1024, path);
#else
	getcwd(path, 1024);
#endif
	//printf("Path: %s\n", path);
	logD("Path: %s\n", path);
	strcat(path, "/..");
	//createFileTree("X:\\Github\\CADlib", &fileTree);
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
	//addJob(jobbedOpenFile, (void*)"x:\\NewProjects\\OdinEditor2\\FileTree.h");
	//openFile("~/test.h");

	g->ctx->style.edit.selected_normal.a = 200;
	g->ctx->style.edit.selected_normal.b = 200;
	//initLayout();
	View* view = createSplitViewPixel(createView(drawFileTree, "File Tree"), createView(drawTextEditor, "Text Editor"), false, true, 300);

	arrayTest();

	//glfwWindowShouldClose can be set from within nuklear_glfw_gl3.h keypress handler
	while (!glfwWindowShouldClose(g->win)) {
		flushLog();
		glfwPollEvents();
		nk_glfw3_new_frame();
		int width;
		int height;
		glfwGetWindowSize(g->win, &width, &height);

		nk_begin(g->ctx, "TEST", nk_rect(0, 0, width, height), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR);
		drawMainMenu();
		drawLayout(view, width, height);
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
