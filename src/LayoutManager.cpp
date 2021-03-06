#include "LayoutManager.h"
#include "TextEditor.h"
#include "FileTree.h"
#include "Array.h"
#include "NuklearAndConfig.h"
#include "Globals.h"
#include <limits.h>

#define RESIZE_HANDLE_SIZE 10

struct View;

enum ViewType {
	VIEW_SINGLE,
	VIEW_SPLIT_HORIZONTAL_PERCENTAGE,//The split between the two views is based on a percentage i.e. 50% for the left view %50 for the right view
	VIEW_SPLIT_HORIZONTAL_PIXEL_LEFT,//The pixel based left split is where the width of the left view is specified, and the rest is for the right view
	VIEW_SPLIT_HORIZONTAL_PIXEL_RIGHT,
	VIEW_SPLIT_VERTICAL_PERCENTAGE,
	VIEW_SPLIT_VERTICAL_PIXEL_TOP,
	VIEW_SPLIT_VERTICAL_PIXEL_BOTTOM
};

struct ViewSingle {
    const char* title;
	LayoutFunc draw;
};

struct ViewSplit {
	View* view1;
	View* view2;
};

struct View {
	ViewType type;
	union {
		ViewSingle single;
		ViewSplit  split;
	};
	union {
		int pixelSplit;
		float percentageSplit;
	};
};

View* createView(LayoutFunc draw, const char* title) {
	View* view = new View();
	view->single.draw = draw;
	view->single.title = title;
	return view;
}

View* createSplitViewPercentage(View* view1, View* view2, bool isVertical, float percentage) {
	View* view = new View();
	if (isVertical) {
		view->type = VIEW_SPLIT_VERTICAL_PERCENTAGE;
	}
	else {
		view->type = VIEW_SPLIT_HORIZONTAL_PERCENTAGE;
	}
	view->split.view1 = view1;
	view->split.view2 = view2;
	view->percentageSplit = percentage;
	return view;
}

View* createSplitViewPixel(View* view1, View* view2, bool isVertical, bool isFirstViewLimited, int pixels) {
	View* view = new View();
	if (isVertical) {
		if (isFirstViewLimited) {
			view->type = VIEW_SPLIT_VERTICAL_PIXEL_TOP;
		}
		else {
			view->type = VIEW_SPLIT_VERTICAL_PIXEL_BOTTOM;
		}
	}
	else {
		if (isFirstViewLimited) {
			view->type = VIEW_SPLIT_HORIZONTAL_PIXEL_LEFT;
		}
		else {
			view->type = VIEW_SPLIT_HORIZONTAL_PIXEL_RIGHT;
		}
	}
	view->split.view1 = view1;
	view->split.view2 = view2;
	view->pixelSplit = pixels;
	return view;
}

void drawLayoutRecursively(View* view, struct nk_rect totalArea) {
	if (view->type == VIEW_SINGLE) {
		nk_layout_space_push(g->ctx, totalArea);
		nk_group_begin(g->ctx, view->single.title, NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER);
		view->single.draw();
		nk_group_end(g->ctx);
	}
	else {
		struct nk_rect area1;
		struct nk_rect area2;
		nk_input* input = &g->ctx->input;

		//Deal with horizontal splits then vertical
		if (view->type == VIEW_SPLIT_HORIZONTAL_PERCENTAGE ||
			view->type == VIEW_SPLIT_HORIZONTAL_PIXEL_LEFT ||
			view->type == VIEW_SPLIT_HORIZONTAL_PIXEL_RIGHT) {

			float split;

			if (view->type == VIEW_SPLIT_HORIZONTAL_PERCENTAGE) {
				split = totalArea.w * view->percentageSplit;
			}
			else if (view->type == VIEW_SPLIT_HORIZONTAL_PIXEL_LEFT) {
				split = view->pixelSplit;
			}
			else if (view->type == VIEW_SPLIT_HORIZONTAL_PIXEL_RIGHT) {
				split = totalArea.w - view->pixelSplit;
			}

			struct nk_rect resizeHandle = nk_rect(totalArea.x + split - RESIZE_HANDLE_SIZE / 2.0f, totalArea.y, RESIZE_HANDLE_SIZE, totalArea.h);

			if (nk_input_is_mouse_hovering_rect(input, resizeHandle)) {
				nk_style_set_cursor(g->ctx, NK_CURSOR_RESIZE_HORIZONTAL);
			}

			bool leftMouseDown = input->mouse.buttons[NK_BUTTON_LEFT].down;
			bool leftMouseClickInResizer = nk_input_has_mouse_click_down_in_rect(input, NK_BUTTON_LEFT, resizeHandle, nk_true);

			if (nk_input_is_mouse_down(input, NK_BUTTON_LEFT) && leftMouseDown && leftMouseClickInResizer) {
				float delta_x = g->ctx->input.mouse.delta.x;

				if (view->type == VIEW_SPLIT_HORIZONTAL_PERCENTAGE) {
					view->percentageSplit = (g->ctx->input.mouse.pos.x - totalArea.x) / totalArea.w;
				}
				else if (view->type == VIEW_SPLIT_HORIZONTAL_PIXEL_LEFT) {
					view->pixelSplit = g->ctx->input.mouse.pos.x - totalArea.x;
				}
				else if (view->type == VIEW_SPLIT_HORIZONTAL_PIXEL_RIGHT) {
					view->pixelSplit = totalArea.w - (g->ctx->input.mouse.pos.x - totalArea.x);
				}
				g->ctx->input.mouse.buttons[NK_BUTTON_LEFT].clicked_pos.x = split - totalArea.x + delta_x;
				g->ctx->input.mouse.buttons[NK_BUTTON_LEFT].clicked_pos.y = g->ctx->input.mouse.pos.y;
			}

			area1.h = totalArea.h;
			area1.y = totalArea.y;
			area1.x = totalArea.x;
			area1.w = split;

			area2.h = totalArea.h;
			area2.y = totalArea.y;
			area2.x = totalArea.x + split;
			area2.w = totalArea.w - split;

		}
		else if (view->type == VIEW_SPLIT_VERTICAL_PERCENTAGE ||
			view->type == VIEW_SPLIT_VERTICAL_PIXEL_TOP ||
			view->type == VIEW_SPLIT_VERTICAL_PIXEL_BOTTOM) {

			float split;

			if (view->type == VIEW_SPLIT_VERTICAL_PERCENTAGE) {
				split = totalArea.w * view->percentageSplit;
			}
			else if (view->type == VIEW_SPLIT_VERTICAL_PIXEL_TOP) {
				split = view->pixelSplit;
			}
			else if (view->type == VIEW_SPLIT_VERTICAL_PIXEL_BOTTOM) {
				split = totalArea.w - view->pixelSplit;
			}

			struct nk_rect resizeHandle = nk_rect(totalArea.x, totalArea.y + split - RESIZE_HANDLE_SIZE / 2.0f, totalArea.w, RESIZE_HANDLE_SIZE);

			if (nk_input_is_mouse_hovering_rect(input, resizeHandle)) {
				nk_style_set_cursor(g->ctx, NK_CURSOR_RESIZE_VERTICAL);
			}

			bool leftMouseDown = input->mouse.buttons[NK_BUTTON_LEFT].down;
			bool leftMouseClickInResizer = nk_input_has_mouse_click_down_in_rect(input, NK_BUTTON_LEFT, resizeHandle, nk_true);

			if (nk_input_is_mouse_down(input, NK_BUTTON_LEFT) && leftMouseDown && leftMouseClickInResizer) {
				float delta_y = g->ctx->input.mouse.delta.y;

				if (view->type == VIEW_SPLIT_VERTICAL_PERCENTAGE) {
					view->percentageSplit = (g->ctx->input.mouse.pos.y - totalArea.y) / totalArea.h;
				}
				else if (view->type == VIEW_SPLIT_VERTICAL_PIXEL_TOP) {
					view->pixelSplit = g->ctx->input.mouse.pos.y - totalArea.y;
				}
				else if (view->type == VIEW_SPLIT_VERTICAL_PIXEL_BOTTOM) {
					view->pixelSplit = totalArea.h - (g->ctx->input.mouse.pos.y - totalArea.y);
				}
				g->ctx->input.mouse.buttons[NK_BUTTON_LEFT].clicked_pos.y = split - totalArea.y + delta_y;
				g->ctx->input.mouse.buttons[NK_BUTTON_LEFT].clicked_pos.x = g->ctx->input.mouse.pos.x;
			}

			area1.h = split;
			area1.y = totalArea.y;
			area1.x = totalArea.x;
			area1.w = totalArea.w;

			area2.h = totalArea.h - split;
			area2.y = totalArea.y + split;
			area2.x = totalArea.x;
			area2.w = totalArea.w;
		}
		else {
			logE("Unknown view type!\n");
			assert(false);
		}

		drawLayoutRecursively(view->split.view1, area1);
		drawLayoutRecursively(view->split.view2, area2);
	}
}

void drawLayout(View* view, int width, int height) {
	nk_layout_space_begin(g->ctx, NK_STATIC, height, INT_MAX);
	drawLayoutRecursively(view, nk_rect(0, 0, width, height));
	nk_layout_space_end(g->ctx);
}
