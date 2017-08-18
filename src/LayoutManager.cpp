#include "LayoutManager.h"
#include "TextEditor.h"
#include "FileTree.h"
struct ViewLayout;

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
	char* title;
	LayoutFunc draw;
};

struct ViewSplit {
	ViewLayout* view1;
	ViewLayout* view2;
};

struct ViewLayout {
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

//A fully layed out view, ready to be drawn
struct View {
	struct nk_rect bounds;
	LayoutFunc draw;
	char* title;
};

static ViewLayout root;
static Array<View> layedoutViews;

void initLayout() {
	arrayInit(&layedoutViews);
}

ViewLayout* createView(LayoutFunc draw, char* title) {
	ViewLayout* view = new ViewLayout();
	view->single.draw = draw;
	view->single.title = title;
	return view;
}

ViewLayout* createSplitViewPercentage(ViewLayout* view1, ViewLayout* view2, bool isVertical, float percentage) {
	ViewLayout* view = new ViewLayout();
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

ViewLayout* createSplitViewPixel(ViewLayout* view1, ViewLayout* view2, bool isVertical, bool isFirstViewLimited, int pixels) {
	ViewLayout* view = new ViewLayout();
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

void layoutViewsRecursively(ViewLayout* view, struct nk_rect totalArea) {
	if (view->type == VIEW_SINGLE) {
		View layedoutView;
		layedoutView.bounds = totalArea;
		layedoutView.draw = view->single.draw;
		layedoutView.title = view->single.title;
		arrayAdd(&layedoutViews, layedoutView);
	}
	else {
		struct nk_rect area1;
		struct nk_rect area2;

		//Deal with horizontal splits
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

			if (abs(g->ctx->input.mouse.pos.x - split - totalArea.x) < 5) {
				g->ctx->style.cursor_type = NK_CURSOR_RESIZE_HORIZONTAL;
				if (nk_input_is_mouse_down(&g->ctx->input, NK_BUTTON_LEFT)) {
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
					g->ctx->input.mouse.buttons[NK_BUTTON_LEFT].clicked_pos.x = split - totalArea.x;
					g->ctx->input.mouse.buttons[NK_BUTTON_LEFT].clicked_pos.y = g->ctx->input.mouse.pos.y;
				}
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

			if ( abs(g->ctx->input.mouse.pos.y - split - totalArea.y) < 5) {
				g->ctx->style.cursor_type = NK_CURSOR_RESIZE_VERTICAL;
				if (nk_input_is_mouse_down(&g->ctx->input, NK_BUTTON_LEFT)) {
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
					g->ctx->input.mouse.buttons[NK_BUTTON_LEFT].clicked_pos.y = split - totalArea.y;
					g->ctx->input.mouse.buttons[NK_BUTTON_LEFT].clicked_pos.x = g->ctx->input.mouse.pos.x;
				}
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

		layoutViewsRecursively(view->split.view1, area1);
		layoutViewsRecursively(view->split.view2, area2);
	}
}

void layoutViews(ViewLayout* view, int width, int height) {
	layedoutViews.len = 0;//Clear the array
	layoutViewsRecursively(view, nk_rect(0, 0, width, height));
}

void drawLayout(int width, int height) {
	nk_layout_space_begin(g->ctx, NK_STATIC, nk_window_get_content_region(g->ctx).h, INT_MAX);
	for (int i = 0; i < layedoutViews.len; i++) {
		nk_layout_space_push(g->ctx, layedoutViews[i].bounds);
		nk_group_begin(g->ctx, layedoutViews[i].title, NULL);
		layedoutViews[i].draw();
		nk_group_end(g->ctx);
	}
	nk_layout_space_end(g->ctx);
}