#pragma once
#include "Defines.h"

struct View;
typedef void(*LayoutFunc)();

//void initLayout();
void drawLayout(View* view, int width, int height);
EXPORT View* createSplitViewPercentage(View* view1, View* view2, bool isVertical, float percentage);
EXPORT View* createSplitViewPixel(View* view1, View* view2, bool isVertical, bool isFirstViewLimited, int pixels);
EXPORT View* createView(LayoutFunc draw, const char* title);
