#include "Globals.h"
#include "Theme.h"

void setupTheme() {
	//Setup fonts
	{
		char fontsDir[1024];//The directory the program is in
#ifdef _WIN32
		GetModuleFileName(NULL, fontsDir, 1024);
		char* fileName = strrchr(fontsDir, '\\');
		strncpy(fileName, "\\fonts\\", 1024 - (fileName - fontsDir));
#elif __linux__
		{
			int len = readlink("/proc/self/exe", fontsDir, 1024);
			fontsDir[len] = '\0';
			char* fileName = strrchr(fontsDir, '/');
			strncpy(fileName, "/fonts/", 1024 - (fileName - fontsDir));
		}
#else
#error "Unsupported platform!"
#endif
		logI("Fonts Dir: %s\n", fontsDir);
		nk_glfw3_font_stash_begin(&g->theme.atlas);

		char fontPath[1024];
		strcpy(fontPath, fontsDir);
		char* fontFileName = &fontPath[strlen(fontsDir)];

		struct nk_font_config cfg = nk_font_config(0);
		cfg.spacing = nk_vec2i(2, 2);
		cfg.oversample_v = 2;

		strcpy(fontFileName, "CodeNewRoman.ttf");
		g->theme.codeFont = nk_font_atlas_add_from_file(g->theme.atlas, fontPath, 14, &cfg);

		strcpy(fontFileName, "RobotoCondensed-Regular.ttf");
		g->theme.UIFont = nk_font_atlas_add_from_file(g->theme.atlas, fontPath, 16, &cfg);

		nk_glfw3_font_stash_end();
		nk_style_set_font(g->ctx, &g->theme.UIFont->handle);
	}

	//Setup code colors for syntax highlighting
	{
		g->theme.codeColors = new nk_color[TOK_COUNT];

		g->theme.codeColors[TOK_DEFAULT] = nk_rgb(131, 148, 150);
		g->theme.codeColors[TOK_COMMENT] = nk_rgb(38, 139, 210);
		g->theme.codeColors[TOK_IDENTIFIER] = nk_rgb(238, 232, 213);
		g->theme.codeColors[TOK_STRING] = nk_rgb(42, 161, 152);
		g->theme.codeColors[TOK_NUMBER] = nk_rgb(42, 161, 152);
		g->theme.codeColors[TOK_RESERVED] = nk_rgb(203, 75, 22);
	}
}

void destroyTheme() {
	delete[] g->theme.codeColors;
	//Font atlas cleanup is handled by nk_glfw3_shutdown()
}