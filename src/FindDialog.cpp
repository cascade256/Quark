#include "FindDialog.h"

#define MAX_SEARCH_LEN 1024

void drawFindDialog(MyOpenFile* file, struct nk_vec2 pos) {
    static nk_window* popup;
    static char searchString[MAX_SEARCH_LEN];
    static int used = 0;

    nk_style* style = &g->ctx->style;
    float row_height = style->font->height + style->button.padding.y * 2;

    struct nk_rect area = nk_rect(pos.x, pos.y, 200, row_height + style->window.popup_padding.y * 2);
    if(nk_popup_begin(g->ctx, NK_POPUP_DYNAMIC, "Find and Replace", NK_WINDOW_NO_SCROLLBAR, area)) {
        nk_layout_row_static(g->ctx, row_height, 200, 1);
        //nk_label_colored(g->ctx, "Hallo", NK_TEXT_LEFT, nk_rgb(255, 255, 255));

        nk_edit_string(g->ctx, NK_EDIT_SIMPLE, searchString, &used, MAX_SEARCH_LEN, nk_filter_ascii);
        nk_popup_end(g->ctx);
    }
}
