#include "LayoutManager.h"
#include "TextEditor.h"
#include "FileTree.h"
struct View;

enum ViewType {
	VIEW_SINGLE,
	VIEW_SPLIT_HORIZONTAL,
	VIEW_SPLIT_VERTICAL
};

struct ViewSingle {
	char* title;
	LayoutFunc draw;
};

struct ViewSplitHorizontal {
	View* left;
	View* right;
};

struct ViewSplitVertical {
	View* top;
	View* bottom;
};

struct View {
	ViewType type;
	union {
		ViewSingle single;
		ViewSplitHorizontal horizontal;
		ViewSplitVertical vertical;
	};
};


static View root;

View* createHorizontalSplitView(View* left, View* right) {
	View* view = new View();
	view->type = VIEW_SPLIT_HORIZONTAL;
	view->horizontal.left = left;
	view->horizontal.right = right;
	return view;
}

View* createVerticalSplitView(View* top, View* bottom) {
	View* view = new View();
	view->type = VIEW_SPLIT_VERTICAL;
	view->vertical.top = top;
	view->vertical.bottom = bottom;
	return view;
}

View* createView(LayoutFunc draw, char* title) {
	View* view = new View();
	view->single.draw = draw;
	view->single.title = title;
	return view;
}


void layoutTestDraw() {
	struct nk_rect region = nk_window_get_content_region(g->ctx);
	//nk_layout_row_begin(g->ctx, NK_STATIC, region.h, 2);
	nk_layout_row_dynamic(g->ctx, 100, 2);
	nk_button_text(g->ctx, "Hallo world!", 12);
	nk_button_text(g->ctx, "Hello world!", 12);
	//nk_layout_row_end(g->ctx);

}

void initLayout() {
	View* fileTreeView = new View();
	fileTreeView->type = VIEW_SINGLE;
	fileTreeView->single.draw = layoutTestDraw;
	fileTreeView->single.title = "File Tree";

	View* textEditorView = new View();
	textEditorView->type = VIEW_SINGLE;
	textEditorView->single.draw = layoutTestDraw;
	textEditorView->single.title = "Text Editor";

	root.type = VIEW_SPLIT_VERTICAL;
	root.vertical.top = fileTreeView;
	root.vertical.bottom = textEditorView;
}

void drawLayoutRecursively(const View* view, struct nk_rect area) {
	switch (view->type) {
	case VIEW_SINGLE:
	{
		nk_begin(g->ctx, view->single.title, area, NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR);
		view->single.draw();
		nk_end(g->ctx);
		break;
	}
	case VIEW_SPLIT_HORIZONTAL:
	{
		if (abs(g->ctx->input.mouse.pos.x - area.x - area.w / 2) < 5) {
			//logD("The mouse is close the a horizontal line!\n");
		}
		drawLayoutRecursively(view->horizontal.left, nk_rect(area.x, area.y, area.w / 2, area.h));
		drawLayoutRecursively(view->horizontal.right, nk_rect(area.x + area.w / 2, area.y, area.w / 2, area.h));
		break;
	}	
	case VIEW_SPLIT_VERTICAL:
	{
		if (abs(g->ctx->input.mouse.pos.y - area.y - area.h / 2) < 5) {
			//logD("The mouse is close the a vertical line!\n");
		}
		drawLayoutRecursively(view->vertical.top, nk_rect(area.x, area.y, area.w, area.h / 2));
		drawLayoutRecursively(view->vertical.bottom, nk_rect(area.x, area.y + area.h / 2, area.w, area.h / 2));
		break;
	}
	default:
		logE("Unknown view type!\n");
		assert(false);
		break;
	}
}

void drawLayout(View* view, int width, int height) {
	drawLayoutRecursively(view, nk_rect(0, 0, width, height));
}