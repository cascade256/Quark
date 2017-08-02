#include "FindDialog.h"

#define MAX_SEARCH_LEN 1024

struct TextPos {
	int line;
	int col;
};

static TextPos* matches;
static int matchesCapacity;
static int numMatches;
static int currentMatch;
static bool searchIsDirty;
static char lastSearch[MAX_SEARCH_LEN];


struct SearchArgs {
	const Array<TextLine>* lines;
	const char* str;
	int strLen;
};

void jobbedSearch(void* args);

bool drawFindDialog(MyOpenFile* file, struct nk_vec2 pos) {
    static nk_window* popup;
    static char searchString[MAX_SEARCH_LEN];
    static int used = 0;

	bool shouldClose = false;

    nk_style* style = &g->ctx->style;
    float row_height = style->font->height + style->button.padding.y * 2;

    struct nk_rect area = nk_rect(pos.x, pos.y, 200, row_height + style->window.popup_padding.y * 2);
    if(nk_popup_begin(g->ctx, NK_POPUP_STATIC, "Find and Replace", NK_WINDOW_NO_SCROLLBAR, area)) {
        nk_layout_row_static(g->ctx, row_height, 200, 1);
        //nk_label_colored(g->ctx, "Hallo", NK_TEXT_LEFT, nk_rgb(255, 255, 255));

        nk_edit_string(g->ctx, NK_EDIT_SIMPLE, searchString, &used, MAX_SEARCH_LEN, nk_filter_ascii);
		if (nk_input_is_key_pressed(&g->ctx->input, NK_KEY_ENTER)) {
			//Check to see if the search term has been changed
			if (strcmp(searchString, lastSearch) == false) {
				if (!searchIsDirty && numMatches > 0) {
					currentMatch++;
					if (currentMatch == numMatches) {
						currentMatch = 0;
					}
					file->edit.cursor.line = matches[currentMatch].line;
					file->edit.cursor.col = matches[currentMatch].col;
					file->edit.cursorMoved = true;
				}
			}
			else {
				//If it has, update lastSearch and rerun the search
				strncpy(lastSearch, searchString, MAX_SEARCH_LEN);
				SearchArgs* args = new SearchArgs();
				args->lines = &file->edit.lines;
				args->str = searchString;
				args->strLen = used;
				searchIsDirty = true;
				jobbedSearch((void*)args);

				currentMatch = 0;
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
	numMatches = 0;
	matchesCapacity = 10;
	if (matches != NULL) {
		delete [] matches;
	}
	matches = new TextPos[matchesCapacity];

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

						//Expand the array if there is not enough room
						if (numMatches >= matchesCapacity) {
							TextPos* oldMatches = matches;
							matchesCapacity = matchesCapacity * 2;
							matches = new TextPos[matchesCapacity];
							memcpy(matches, oldMatches, numMatches * sizeof(TextPos));
						}
						//Add the match to the array
						matches[numMatches].line = i;
						matches[numMatches].col = x;
						numMatches++;
					}
					x++;
					j++;
				}
			}
			x++;
		}
	}
	logI("%i matches found \n", numMatches);
	searchIsDirty = false;
}

void jobbedSearch(void* args) {
	SearchArgs* sa = (SearchArgs*)args;
	search(sa->lines, sa->str, sa->strLen);
	delete sa;
}