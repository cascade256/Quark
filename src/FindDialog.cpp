#include "FindDialog.h"
#include "Globals.h"


struct SearchArgs {
	const Array<TextLine>* lines;
	const char* str;
	int strLen;
};

void jobbedSearch(void* args);

bool drawFindDialog(MyOpenFile* file, struct nk_vec2 pos) {
	FindDialogCache* cache = &g->findCache;
	Array<TextPos>* matches = &g->findCache.matches;
	bool shouldClose = false;

    nk_style* style = &g->ctx->style;
    float row_height = style->font->height + style->button.padding.y * 2;

    struct nk_rect area = nk_rect(pos.x, pos.y, 200, row_height + style->window.popup_padding.y * 2);
    if(nk_popup_begin(g->ctx, NK_POPUP_STATIC, "Find and Replace", NK_WINDOW_NO_SCROLLBAR, area)) {
        nk_layout_row_static(g->ctx, row_height, 200, 1);
        //nk_label_colored(g->ctx, "Hallo", NK_TEXT_LEFT, nk_rgb(255, 255, 255));

        nk_edit_string(g->ctx, NK_EDIT_SIMPLE, cache->currentSearch, &cache->used, MAX_SEARCH_LEN, nk_filter_ascii);
		if (nk_input_is_key_pressed(&g->ctx->input, NK_KEY_ENTER)) {
			//Check to see if the search term has been changed
			if (strcmp(cache->currentSearch, cache->lastSearch) == false) {
				if (!cache->searchIsDirty && cache->matches.len > 0) {
					cache->currentMatch++;
					if (cache->currentMatch == matches->len) {
						cache->currentMatch = 0;
					}
					file->edit.cursor.line = matches->data[cache->currentMatch].line;
					file->edit.cursor.col = matches->data[cache->currentMatch].col;
					file->edit.cursorMoved = true;
				}
			}
			else {
				//If it has, update lastSearch and rerun the search
				strncpy(cache->lastSearch, cache->currentSearch, MAX_SEARCH_LEN);
				SearchArgs* args = new SearchArgs();
				args->lines = &file->edit.lines;
				args->str = cache->currentSearch;
				args->strLen = cache->used;
				cache->searchIsDirty = true;
				jobbedSearch((void*)args);

				cache->currentMatch = 0;
			}

			logD("Enter key down\n");
		}
		if (nk_input_is_key_pressed(&g->ctx->input, NK_KEY_ESCAPE)) {
			nk_popup_close(g->ctx);
			shouldClose = true;
		}
        nk_popup_end(g->ctx);
    }
	return !shouldClose;
}



void search(const Array<TextLine>* lines, const char* str, const int strLen) {
	Array<TextPos>* matches = &g->findCache.matches;
	arrayClear(matches);

	for (int i = 0; i < lines->len; i++) {
		int x = 0;
		TextLine* line = &lines->data[i];
		while (x < line->text.len) {
			if (line->text.data[x] == str[0]) {
				int j = 0;
				while (line->text.data[x] == str[j]) {
					//Check to see if we are at the end of the string
					if (j == strLen - 1) {
						//We found a match!
						TextPos pos;
						pos.line = i;
						pos.col = x;
						arrayAdd(matches, pos);
					}
					x++;
					j++;
				}
			}
			x++;
		}
	}
	logI("%i matches found \n", matches->len);
	g->findCache.searchIsDirty = false;
}

void jobbedSearch(void* args) {
	SearchArgs* sa = (SearchArgs*)args;
	search(sa->lines, sa->str, sa->strLen);
	delete sa;
}
