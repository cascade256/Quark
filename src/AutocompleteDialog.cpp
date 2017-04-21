#include "AutocompleteDialog.h"

nk_window* popup;

void drawAutocompleteDialog(struct nk_vec2 pos, AutoCompleteData* data, bool* active) {
	nk_style* style = &g->ctx->style;
	
	float row_height = style->font->height + style->button.padding.y * 2;
	struct nk_rect area = nk_rect(pos.x, pos.y, 200, data->numOptions * row_height + style->window.popup_padding.y * 2);

	//Check if there was a click outside the dialog
	if (nk_input_has_mouse_click(&g->ctx->input, NK_BUTTON_LEFT) && !
		nk_input_mouse_clicked(&g->ctx->input, NK_BUTTON_LEFT, area)) {
		*active = false;
		return;
	}

	if (nk_my_popup_begin(g->ctx, popup, NK_POPUP_DYNAMIC, "Autocomplete", NK_WINDOW_NO_SCROLLBAR, area)) {

		for (int i = 0; i < data->numOptions; i++) {
			nk_layout_row_static(g->ctx, row_height, 200, 1);
			nk_label_colored(g->ctx, data->options[i].title, NK_TEXT_LEFT, nk_rgb(255, 255, 255));
		}
		nk_my_popup_end(g->ctx);
	}

}

void destroyAutoCompleteData(AutoCompleteData* data) {
	for (int i = 0; i < data->numOptions; i++) {
		delete[] data->options[i].complete;
		delete[] data->options[i].title;
		delete[] data->options[i].desc;
	}
	delete[] data->options;
	delete data;
}