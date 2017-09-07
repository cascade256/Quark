#pragma once
#include "Array.h"
#include "NuklearAndConfig.h"
#include "Globals.h"
#include <limits.h>
struct View;
typedef void(*LayoutFunc)();

//void initLayout();
void drawLayout(View* view, int width, int height);
View* createSplitViewPercentage(View* view1, View* view2, bool isVertical, float percentage);
View* createSplitViewPixel(View* view1, View* view2, bool isVertical, bool isFirstViewLimited, int pixels);
View* createView(LayoutFunc draw, const char* title);
