#pragma once
#include "Array.h"
#include "NuklearAndConfig.h"
#include "Globals.h"

struct View;
typedef void(*LayoutFunc)();

//void initLayout();
void drawLayout(View* view, int width, int height);

View* createHorizontalSplitView(View* left, View* right);
View* createVerticalSplitView(View* top, View* bottom);
View* createView(LayoutFunc draw, char* title);