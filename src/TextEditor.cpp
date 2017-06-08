#include "TextEditor.h"

static bool findDialogOpen = false;

void drawTextEditor(nk_context* ctx, OpenFiles* files) {

	struct nk_rect region;
	region = nk_window_get_content_region(ctx);

	if (files->len > 0) {

		if (nk_group_begin(ctx, "tabs", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
			int tabHeight = 20;
			nk_layout_row_dynamic(ctx, tabHeight, files->len + 1);
			for (int i = 0; i < files->len; i++) {
				if (files->openFiles[i].unsaved) {
					files->openFiles[i].name[files->openFiles[i].nameLen - 1] = '*';
				}
				else {
					files->openFiles[i].name[files->openFiles[i].nameLen - 1] = '\0';
				}
				if (nk_button_label(ctx, files->openFiles[i].name)) {
					g->activeFileIndex = i;
				}
			}
			region = nk_window_get_content_region(ctx);
			nk_layout_row_static(ctx, region.h - tabHeight, region.w, 1);
			//printf("Active file index: %i\n", activeFileIndex);


			//This section needs to be locked because when a new file is added, the array of open files is memcopied to a new array and the
			//old one is deleted. If this happens while nk_my_text_editor() is running, f->edit will point to freed memory. Similar for 
			//drawFindDialog().
			mtx_lock(&g->filesMutex);

			MyOpenFile* f = &files->openFiles[g->activeFileIndex];
			nk_my_text_editor(ctx, NK_EDIT_CLIPBOARD | NK_EDIT_SIMPLE | NK_EDIT_MULTILINE | NK_EDIT_ALLOW_TAB, &f->edit, nk_filter_default);

			if(findDialogOpen) {
				findDialogOpen = drawFindDialog(&files->openFiles[g->activeFileIndex], nk_vec2(region.w - 200, tabHeight));
			}

			mtx_unlock(&g->filesMutex);

			nk_group_end(ctx);

		}
		if (files->openFiles[g->activeFileIndex].edit.acActive) {
			struct nk_vec2 cursorPos = nk_my_get_cursor_pos(&files->openFiles[g->activeFileIndex].edit, g->ctx->style.font, g->ctx->style.font->height + g->ctx->style.edit.row_padding);
			drawAutocompleteDialog(nk_vec2(cursorPos.x + region.x + 5, cursorPos.y + region.y + 20),
				files->openFiles[g->activeFileIndex].edit.acData,
				&(files->openFiles[g->activeFileIndex].edit.acActive));

		}
	}
}

void saveActiveFile(OpenFiles* files) {
	if (files->len > 0) {
		assert(g->activeFileIndex < files->len);
		saveFile(&files->openFiles[g->activeFileIndex]);
	}

}

void showSearchDialog() {
	findDialogOpen = true;
}
