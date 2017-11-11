#pragma once
//#include "NuklearAndConfig.h"
#include "nuklear/nuklear_glfw_gl3.h"

struct Theme {
	nk_font_atlas* atlas;
	nk_font* UIFont;
	nk_font* codeFont;
	nk_color* codeColors;
};

void setupTheme();
void destroyTheme();
