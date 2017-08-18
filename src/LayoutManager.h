#pragma once
#include "Array.h"
#include "NuklearAndConfig.h"
#include "Globals.h"

struct ViewLayout;
typedef void(*LayoutFunc)();

void initLayout();
void layoutViews(ViewLayout* view, int width, int height);
void drawLayout(int width, int height);
ViewLayout* createSplitViewPercentage(ViewLayout* view1, ViewLayout* view2, bool isVertical, float percentage);
ViewLayout* createSplitViewPixel(ViewLayout* view1, ViewLayout* view2, bool isVertical, bool isFirstViewLimited, int pixels);
ViewLayout* createView(LayoutFunc draw, char* title);