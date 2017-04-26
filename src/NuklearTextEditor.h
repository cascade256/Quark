#pragma once
#include "PluginProtocol.h"
#include "nuklear/nuklear.h"
#include <stdio.h>
#include <string.h>
struct nk_my_text_edit {
	struct nk_my_clipboard clip;
	struct TextBuffer buffer;
	nk_plugin_filter filter;
	struct nk_vec2 scrollbar;

	nk_my_vec2i cursor;//line, then column
	nk_my_vec2i select_start;
	nk_my_vec2i select_end;
	unsigned char mode;
	unsigned char cursor_at_end_of_line;
	unsigned char initialized;
	unsigned char has_preferred_x;
	unsigned char single_line;
	unsigned char active;
	unsigned char padding1;
	float preferred_x;
	struct nk_text_undo_state undo;

	nk_color* colorTable;
	Colorize_Func colorize = NULL;

	AutoComplete_Func autocomplete = NULL;
	AutoCompleteData* acData;
	bool acActive;
};

NK_API nk_flags
nk_my_text_editor(struct nk_context *ctx, nk_flags flags,
	struct nk_my_text_edit *edit, nk_plugin_filter filter);

NK_API struct nk_vec2
nk_my_get_cursor_pos(struct nk_my_text_edit* edit, const struct nk_user_font* font, float row_height);

/*-----------------------------------------------------------------
*
*						My Text Editor
*
*----------------------------------------------------------------*/
#ifdef NK_IMPLEMENTATION

#define NK_MY_TEXT_HAS_SELECTION(s)   (!((s)->select_start.x == (s)->select_end.x && (s)->select_start.y == (s)->select_end.y))

NK_INTERN nk_flags
nk_my_do_edit(nk_flags *state, struct nk_command_buffer *out,
	struct nk_rect bounds, nk_flags flags, nk_plugin_filter filter,
	struct nk_my_text_edit *edit, const struct nk_style_edit *style,
	struct nk_input *in, const struct nk_user_font *font);


NK_API nk_flags
nk_my_text_editor(struct nk_context *ctx, nk_flags flags,
	struct nk_my_text_edit *edit, nk_plugin_filter filter)
{
	struct nk_window *win;
	struct nk_style *style;
	struct nk_input *in;

	enum nk_widget_layout_states state;
	struct nk_rect bounds;

	nk_flags ret_flags = 0;
	unsigned char prev_state;
	nk_hash hash;

	/* make sure correct values */
	NK_ASSERT(ctx);
	NK_ASSERT(edit);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout)
		return 0;

	win = ctx->current;
	style = &ctx->style;
	state = nk_widget(&bounds, ctx);
	if (!state) return state;
	in = (win->layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;

	/* check if edit is currently hot item */
	hash = win->edit.seq++;
	if (win->edit.active && hash == win->edit.name) {
		if (flags & NK_EDIT_NO_CURSOR)
		{
			edit->cursor.x = edit->buffer.len;
			edit->cursor.y = edit->buffer.lines[edit->buffer.len - 1].len;
		}
		/*if (!(flags & NK_EDIT_SELECTABLE)) {
			edit->select_start = edit->cursor;
			edit->select_end = edit->cursor;
		}*/
		if (flags & NK_EDIT_CLIPBOARD)
			edit->clip = ctx->my_clip;
	}

	filter = (!filter) ? nk_filter_default : filter;
	prev_state = (unsigned char)edit->active;
	in = (flags & NK_EDIT_READ_ONLY) ? 0 : in;
	ret_flags = nk_my_do_edit(&ctx->last_widget_state, &win->buffer, bounds, flags,
		filter, edit, &style->edit, in, style->font);

	if (ctx->last_widget_state & NK_WIDGET_STATE_HOVER)
		ctx->style.cursor_active = ctx->style.cursors[NK_CURSOR_TEXT];
	if (edit->active && prev_state != edit->active) {
		/* current edit is now hot */
		win->edit.active = nk_true;
		win->edit.name = hash;
	}
	else if (prev_state && !edit->active) {
		/* current edit is now cold */
		win->edit.active = nk_false;
	}
	return ret_flags;
}

NK_INTERN void
nk_my_on_text_change(nk_my_text_edit* state) {
	if (state->acActive) {
		state->autocomplete(state->acData, &state->buffer);
		printf("Num Completions: %i\n", state->acData->numOptions);
	}
}

NK_INTERN float
nk_my_textedit_text_width(const TextLine* line, int start, int end, const nk_user_font* font) {
	nk_rune unicode;

	int i = 0;
	int glyphLen = 0;
	float width = 0;
	while (i < line->len) {
		glyphLen = nk_utf_decode(line->text + i, &unicode, line->len);
		width += font->width(font->userdata, font->height, line->text + i, glyphLen);
		i += glyphLen;
	}
	return width;
}

NK_INTERN void
nk_my_textedit_layout_row(struct nk_text_edit_row *r, struct nk_my_text_edit *edit,
	int lineIndex, float row_height, const struct nk_user_font *font)
{
	nk_rune unicode;
	TextLine line = edit->buffer.lines[lineIndex];

	int i = 0;
	int numGlyphs = 0;
	int glyphLen = 0;
	while (i < line.len) {
		glyphLen = nk_utf_decode(line.text + i, &unicode, line.len);
		i += glyphLen;
		numGlyphs++;
	}

	r->x0 = 0.0f;
	r->x1 = numGlyphs;
	r->baseline_y_delta = row_height;
	r->ymin = 0.0f;
	r->ymax = row_height;
	r->num_chars = numGlyphs;
}


#ifdef ALDSLEIUWHIUNCSEIOUWFBH
NK_API void
nk_my_textedit_undo(struct nk_my_text_edit *state)
{
	struct nk_text_undo_state *s = &state->undo;
	struct nk_text_undo_record u, *r;
	if (s->undo_point == 0)
		return;

	/* we need to do two things: apply the undo record, and create a redo record */
	u = s->undo_rec[s->undo_point - 1];
	r = &s->undo_rec[s->redo_point - 1];
	r->char_storage = -1;

	r->insert_length = u.delete_length;
	r->delete_length = u.insert_length;
	r->where = u.where;

	if (u.delete_length)
	{
		/*   if the undo record says to delete characters, then the redo record will
		need to re-insert the characters that get deleted, so we need to store
		them.
		there are three cases:
		- there's enough room to store the characters
		- characters stored for *redoing* don't leave room for redo
		- characters stored for *undoing* don't leave room for redo
		if the last is true, we have to bail */
		if (s->undo_char_point + u.delete_length >= NK_TEXTEDIT_UNDOCHARCOUNT) {
			/* the undo records take up too much character space; there's no space
			* to store the redo characters */
			r->insert_length = 0;
		}
		else {
			int i;
			/* there's definitely room to store the characters eventually */
			while (s->undo_char_point + u.delete_length > s->redo_char_point) {
				/* there's currently not enough room, so discard a redo record */
				nk_textedit_discard_redo(s);
				/* should never happen: */
				if (s->redo_point == NK_TEXTEDIT_UNDOSTATECOUNT)
					return;
			}

			r = &s->undo_rec[s->redo_point - 1];
			r->char_storage = (short)(s->redo_char_point - u.delete_length);
			s->redo_char_point = (short)(s->redo_char_point - u.delete_length);

			/* now save the characters */
			for (i = 0; i < u.delete_length; ++i)
				s->undo_char[r->char_storage + i] =
				nk_str_rune_at(&state->string, u.where + i);
		}
		/* now we can carry out the deletion */
		nk_str_delete_runes(&state->string, u.where, u.delete_length);
	}

	/* check type of recorded action: */
	if (u.insert_length) {
		/* easy case: was a deletion, so we need to insert n characters */
		nk_str_insert_text_runes(&state->string, u.where,
			&s->undo_char[u.char_storage], u.insert_length);
		s->undo_char_point = (short)(s->undo_char_point - u.insert_length);
	}
	state->cursor = (short)(u.where + u.insert_length);

	s->undo_point--;
	s->redo_point--;
}

NK_API void
nk_my_textedit_redo(struct nk_my_text_edit *state)
{
	struct nk_text_undo_state *s = &state->undo;
	struct nk_text_undo_record *u, r;
	if (s->redo_point == NK_TEXTEDIT_UNDOSTATECOUNT)
		return;

	/* we need to do two things: apply the redo record, and create an undo record */
	u = &s->undo_rec[s->undo_point];
	r = s->undo_rec[s->redo_point];

	/* we KNOW there must be room for the undo record, because the redo record
	was derived from an undo record */
	u->delete_length = r.insert_length;
	u->insert_length = r.delete_length;
	u->where = r.where;
	u->char_storage = -1;

	if (r.delete_length) {
		/* the redo record requires us to delete characters, so the undo record
		needs to store the characters */
		if (s->undo_char_point + u->insert_length > s->redo_char_point) {
			u->insert_length = 0;
			u->delete_length = 0;
		}
		else {
			int i;
			u->char_storage = s->undo_char_point;
			s->undo_char_point = (short)(s->undo_char_point + u->insert_length);

			/* now save the characters */
			for (i = 0; i < u->insert_length; ++i) {
				s->undo_char[u->char_storage + i] =
					nk_str_rune_at(&state->string, u->where + i);
			}
		}
		nk_str_delete_runes(&state->string, r.where, r.delete_length);
	}

	if (r.insert_length) {
		/* easy case: need to insert n characters */
		nk_str_insert_text_runes(&state->string, r.where,
			&s->undo_char[r.char_storage], r.insert_length);
	}
	state->cursor = r.where + r.insert_length;

	s->undo_point++;
	s->redo_point++;
}
#endif

NK_INTERN int
nk_my_bytes_to_glyph(TextLine* line, int cursor) {
	int glyph_len;
	nk_rune unicode;
	int numBytes = 0;
	int numGlyphs = 0;
	while (numBytes < line->len && numGlyphs < cursor) {
		glyph_len = nk_utf_decode(line->text, &unicode, line->len - numBytes);
		numBytes += glyph_len;
		numGlyphs++;
	}
	return numBytes;
}

NK_INTERN void
nk_my_textbuffer_remove_line(struct TextBuffer* buffer, int i) {
	delete[] buffer->lines[i].text;
	delete[] buffer->lines[i].colors;
	NK_MEMCPY(&buffer->lines[i], &buffer->lines[i + 1], (buffer->len - i) * sizeof(TextLine));
	buffer->len--;
}

NK_INTERN void
nk_my_textline_resize(struct TextLine* line, int newSize, int newNumGlyphs) {
	char* oldText = line->text;
	char* oldColors = line->colors;
	line->capacity = newSize;
	line->text = new char[line->capacity];
	line->colorCapacity = newNumGlyphs;
	line->colors = new char[line->colorCapacity];
	NK_MEMCPY(line->text, oldText, line->len);
	NK_MEMCPY(line->colors, oldColors, line->numGlyphs);
	delete[] oldText;
	delete[] oldColors;
}

NK_INTERN void
nk_my_textline_concat(struct TextLine* dest, struct TextLine* src) {
	if (dest->len + src->len > dest->capacity ||
		dest->numGlyphs + src->numGlyphs > dest->colorCapacity) {
		nk_my_textline_resize(dest, dest->len + src->len, dest->numGlyphs + src->numGlyphs);
	}
	NK_MEMCPY(&dest->text[dest->len], src->text, src->len);
	NK_MEMCPY(&dest->colors[dest->numGlyphs], src->colors, src->numGlyphs);
	dest->len += src->len;
	dest->numGlyphs += src->numGlyphs;
}


NK_API void
nk_my_textedit_select_all(struct nk_my_text_edit *state)
{
	NK_ASSERT(state);
	state->select_start.x = 0;
	state->select_start.y = 0;
	state->select_end.x = state->buffer.len - 1;
	state->select_end.y = state->buffer.lines[state->buffer.len - 1].numGlyphs;
}

NK_INTERN void
nk_my_textedit_clamp(struct nk_my_text_edit *state)
{
	/* make the selection/cursor state valid if client altered the string */
	int n = state->buffer.len;
	//If select_start and select_end do not equal each other
	if (NK_MY_TEXT_HAS_SELECTION(state)) {
		if (state->select_start.x >= n) {
			state->select_start.x = n - 1;
		}
		TextLine line = state->buffer.lines[state->select_start.x];
		if (state->select_start.y > line.numGlyphs) {
			state->select_start.y = line.numGlyphs;
		}

		if (state->select_end.x >= n) {
			state->select_end.x = n - 1;
		}
		line = state->buffer.lines[state->select_end.x];
		if (state->select_end.y > line.numGlyphs) {
			state->select_end.y = line.numGlyphs;
		}

		/* if clamping forced them to be equal, move the cursor to match */
		if (state->select_start.x == state->select_end.x &&
			state->select_start.y == state->select_end.y) {
			state->cursor = state->select_start;
		}
	}
	if (state->cursor.x >= n) {
		state->cursor.x = n - 1;
	}
	if (state->cursor.x < 0) {
		state->cursor.x = 0;
	}
	TextLine line = state->buffer.lines[state->cursor.x];
	if (state->cursor.y > line.numGlyphs) {
		state->cursor.y = line.numGlyphs;
	}
	if (state->cursor.y < 0) {
		state->cursor.y = 0;
	}
}

NK_INTERN void
nk_my_textedit_prep_selection_at_cursor(struct nk_my_text_edit *state)
{
	/* update selection and cursor to match each other */
	if (!NK_MY_TEXT_HAS_SELECTION(state))
		state->select_start = state->select_end = state->cursor;
	else state->cursor = state->select_end;
}

NK_INTERN void
nk_my_textedit_sortselection(struct nk_my_text_edit *state)
{
	/* canonicalize the selection so start <= end */
	if (state->select_end.x < state->select_start.x) {
		nk_my_vec2i temp = state->select_end;
		state->select_end = state->select_start;
		state->select_start = temp;
	}
	else if (state->select_end.x == state->select_start.x &&
		state->select_end.y < state->select_start.y) {
		nk_my_vec2i temp = state->select_end;
		state->select_end = state->select_start;
		state->select_start = temp;
	}
}

NK_INTERN void
nk_my_textedit_move_to_first(struct nk_my_text_edit *state)
{
	/* move cursor to first character of selection */
	if (NK_MY_TEXT_HAS_SELECTION(state)) {
		nk_my_textedit_sortselection(state);
		state->cursor = state->select_start;
		state->select_end = state->select_start;
		state->has_preferred_x = 0;
	}
}

NK_INTERN void
nk_my_textedit_move_to_last(struct nk_my_text_edit *state)
{
	/* move cursor to last character of selection */
	if (NK_MY_TEXT_HAS_SELECTION(state)) {
		nk_my_textedit_sortselection(state);
		nk_my_textedit_clamp(state);
		state->cursor = state->select_end;
		state->select_start = state->select_end;
		state->has_preferred_x = 0;
	}
}

NK_INTERN int
nk_my_is_word_boundary(struct nk_my_text_edit *state, nk_my_vec2i idx)
{
	nk_rune c;
	if (idx.y <= 0) return 1;
	TextLine line = state->buffer.lines[idx.x];
	int byteIndex = nk_my_bytes_to_glyph(&line, idx.y);
	if (nk_utf_decode(line.text + byteIndex, &c, line.len - byteIndex) != 1) return 1;
	return (c == ' ' || c == '\t' || c == 0x3000 || c == ',' || c == ';' ||
		c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' ||
		c == '|');
}

NK_INTERN nk_my_vec2i
nk_my_textedit_move_to_word_previous(struct nk_my_text_edit *state)
{
	nk_my_vec2i c = state->cursor;
	c.y -= 1;
	while (c.y >= 0 && !nk_my_is_word_boundary(state, c))
		--c.y;

	if (c.y < 0)
		c.y = 0;

	return c;
}

NK_INTERN nk_my_vec2i
nk_my_textedit_move_to_word_next(struct nk_my_text_edit *state)
{
	const int len = state->buffer.lines[state->cursor.x].numGlyphs;
	nk_my_vec2i c = state->cursor;
	c.y++;
	while (c.y < len && !nk_my_is_word_boundary(state, c))
		++c.y;

	if (c.y > len)
		c.y = len;

	return c;
}

NK_API void
nk_my_textedit_delete(struct nk_my_text_edit *state, nk_my_vec2i start, nk_my_vec2i end)
{
	/* delete characters while updating undo */
	//nk_textedit_makeundo_delete(state, where, len);

	int startByteIndex = nk_my_bytes_to_glyph(&(state->buffer.lines[start.x]), start.y);
	int endByteIndex = nk_my_bytes_to_glyph(&(state->buffer.lines[end.x]), end.y);

	//If the deletion is all on the same line
	if (start.x == end.x) {
		TextLine* line = &state->buffer.lines[start.x];
		memmove(&line->text[startByteIndex], &line->text[endByteIndex], line->len - endByteIndex);
		memmove(&line->colors[start.y], &line->colors[end.y], line->numGlyphs - end.y);
		line->len -= endByteIndex - startByteIndex;
		line->numGlyphs -= end.y - start.y;

		if (state->colorize) {
			state->colorize(&state->buffer, start.x);
		}
	}
	else {
		TextLine* endLine = &state->buffer.lines[end.x];
		memmove(endLine->text, &endLine->text[endByteIndex], endByteIndex);
		memmove(endLine->colors, &endLine->colors[end.y], end.y);

		state->buffer.lines[start.x].len = startByteIndex;
		state->buffer.lines[start.x].numGlyphs = start.y;


		if (state->colorize) {
			state->colorize(&state->buffer, start.x);
			state->colorize(&state->buffer, end.x);
		}

		//delete any whole lines
		int numLines = end.x - start.x;
		if (numLines > 0) {
			for (int i = 0; i < numLines; i++) {
				delete[] state->buffer.lines[start.x + i + 1].text;
			}
			NK_MEMCPY(&state->buffer.lines[start.x + 1], &state->buffer.lines[end.x], (state->buffer.len - end.x) * sizeof(TextLine));
			state->buffer.len -= numLines;
		}
	}

	state->has_preferred_x = 0;
}

NK_API void
nk_my_textedit_delete_selection(struct nk_my_text_edit *state)
{
	/* delete the section */
	nk_my_textedit_clamp(state);
	nk_my_textedit_sortselection(state);
	if (NK_MY_TEXT_HAS_SELECTION(state)) {
		nk_my_textedit_delete(state, state->select_start, state->select_end);
		state->select_end = state->cursor = state->select_start;
		state->has_preferred_x = 0;
	}
}

NK_INTERN void
nk_my_textedit_insert_glyph(struct TextLine* line, nk_my_vec2i* cursor, const char* text, int glyph_len) {
	int byteIndex = nk_my_bytes_to_glyph(line, cursor->y);
	if (line->capacity < line->len + glyph_len || line->numGlyphs + 1 < line->colorCapacity) {
		nk_my_textline_resize(line, line->len + 10 * glyph_len, line->numGlyphs + 10);
	}

	if (byteIndex == line->len - 1) {
		NK_MEMCPY(&line->text[line->len], text, glyph_len);
	}
	else {
		memmove(&line->text[byteIndex + glyph_len], &line->text[byteIndex], line->len - byteIndex);
		NK_MEMCPY(&line->text[byteIndex], text, glyph_len);
		memmove(&line->colors[cursor->y + 1], &line->colors[cursor->y], line->numGlyphs - cursor->y);
	}
	line->colors[cursor->y] = 0;
	line->len += glyph_len;
	line->numGlyphs++;
	cursor->y++;
	//state->has_preferred_x = 0;
}

NK_API void
nk_my_textedit_text(struct nk_my_text_edit *state, const char *text, int total_len)
{
	nk_rune unicode;
	int glyph_len;
	int text_len = 0;

	NK_ASSERT(state);
	NK_ASSERT(text);
	if (!text || !total_len || state->mode == NK_TEXT_EDIT_MODE_VIEW) return;

	glyph_len = nk_utf_decode(text, &unicode, total_len);
	if (!glyph_len) return;
	while ((text_len < total_len) && glyph_len)
	{
		/* don't insert a backward delete, just process the event */
		if (unicode == 127)
			break;

		TextLine* line = &state->buffer.lines[state->cursor.x];
		if (!NK_MY_TEXT_HAS_SELECTION(state))
		{
			if (state->mode == NK_TEXT_EDIT_MODE_REPLACE) {
				//nk_textedit_makeundo_replace(state, state->cursor, 1, 1);
				//TODO
			}
			nk_my_textedit_insert_glyph(line, &state->cursor, text + text_len, glyph_len);

		}
		else {
			nk_my_textedit_delete_selection(state); /* implicitly clamps */
			nk_my_textedit_insert_glyph(line, &state->cursor, text + text_len, glyph_len);
			//nk_textedit_makeundo_insert(state, state->cursor, 1);
		}

		if (unicode == '\n') {
			TextBuffer* buffer = &state->buffer;
			if (buffer->capacity <= buffer->len) {
				TextLine* oldLines = buffer->lines;
				buffer->capacity += 10;
				buffer->lines = new TextLine[buffer->capacity];
				NK_MEMCPY(buffer->lines, oldLines, buffer->len * sizeof(TextLine));
				delete[] oldLines;
			}

			if (state->cursor.x < buffer->len - 1) {
				memmove(&buffer->lines[state->cursor.x + 2],
					&buffer->lines[state->cursor.x + 1],
					(buffer->len - state->cursor.x - 1) * sizeof(TextLine));
			}
			TextLine* line = &buffer->lines[state->cursor.x];
			int byteIndex = nk_my_bytes_to_glyph(line, state->cursor.y);
			TextLine* newLine = &buffer->lines[state->cursor.x + 1];
			newLine->capacity = line->len - byteIndex + 10;
			newLine->numGlyphs = line->numGlyphs - state->cursor.y;
			newLine->colorCapacity = line->numGlyphs + 10;
			newLine->len = line->len - byteIndex;
			newLine->text = new char[newLine->capacity];
			newLine->colors = new char[newLine->colorCapacity];
			NK_MEMCPY(newLine->text, &line->text[byteIndex], newLine->len);
			line->len = byteIndex;
			line->numGlyphs = state->cursor.y;

			if (state->colorize) {
				state->colorize(&state->buffer, state->cursor.x);
			}

			buffer->len++;
			state->cursor.x++;
			state->cursor.y = 0;
			assert(buffer->len <= buffer->capacity);
		}

		if (state->colorize) {
			state->colorize(&state->buffer, state->cursor.x);
		}
		text_len += glyph_len;
		glyph_len = nk_utf_decode(text + text_len, &unicode, total_len - text_len);
	}
	nk_my_on_text_change(state);
}


NK_INTERN void
nk_my_textedit_key(struct nk_my_text_edit *state, enum nk_keys key, int shift_mod,
	const struct nk_user_font *font, float row_height)
{
retry:
	switch (key)
	{
	case NK_KEY_NONE:
	case NK_KEY_CTRL:
	case NK_KEY_ENTER:
	case NK_KEY_SHIFT:
	case NK_KEY_TAB:
	case NK_KEY_COPY:
	case NK_KEY_CUT:
	case NK_KEY_PASTE:
	case NK_KEY_MAX:
	default: break;
	case NK_KEY_TEXT_UNDO:
		//nk_my_textedit_undo(state);
		printf("TODO: Implement UNDO\n");
		state->has_preferred_x = 0;
		break;

	case NK_KEY_TEXT_REDO:
		//nk_my_textedit_redo(state);
		printf("TODO: Implement REDO\n");
		state->has_preferred_x = 0;
		break;

	case NK_KEY_AUTOCOMPLETE:
		if (!state->acActive) {
			state->acData->line = state->cursor.x;
			state->acData->col = state->cursor.y;
			state->autocomplete(state->acData, &state->buffer);
			state->acActive = true;
		}
		else {
			state->acActive = false;
		}

		break;

	case NK_KEY_TEXT_SELECT_ALL:
		nk_my_textedit_select_all(state);
		state->has_preferred_x = 0;
		break;

	case NK_KEY_TEXT_INSERT_MODE:
		if (state->mode == NK_TEXT_EDIT_MODE_VIEW)
			state->mode = NK_TEXT_EDIT_MODE_INSERT;
		break;
	case NK_KEY_TEXT_REPLACE_MODE:
		if (state->mode == NK_TEXT_EDIT_MODE_VIEW)
			state->mode = NK_TEXT_EDIT_MODE_REPLACE;
		break;
	case NK_KEY_TEXT_RESET_MODE:
		if (state->mode == NK_TEXT_EDIT_MODE_INSERT ||
			state->mode == NK_TEXT_EDIT_MODE_REPLACE)
			state->mode = NK_TEXT_EDIT_MODE_VIEW;
		break;

	case NK_KEY_LEFT:
		if (shift_mod) {
			nk_my_textedit_clamp(state);
			nk_my_textedit_prep_selection_at_cursor(state);
			/* move selection left */
			if (state->select_end.y > 0)
				--state->select_end.y;
			state->cursor = state->select_end;
			state->has_preferred_x = 0;
		}
		else {
			/* if currently there's a selection,
			* move cursor to start of selection */
			if (NK_MY_TEXT_HAS_SELECTION(state))
				nk_my_textedit_move_to_first(state);
			else if (state->cursor.y > 0)
				--state->cursor.y;
			state->has_preferred_x = 0;
		} break;

	case NK_KEY_RIGHT:
		if (shift_mod) {
			nk_my_textedit_prep_selection_at_cursor(state);
			/* move selection right */
			++state->select_end.y;
			nk_my_textedit_clamp(state);
			state->cursor = state->select_end;
			state->has_preferred_x = 0;
		}
		else {
			/* if currently there's a selection,
			* move cursor to end of selection */
			if (NK_MY_TEXT_HAS_SELECTION(state))
				nk_my_textedit_move_to_last(state);
			else ++state->cursor.y;
			nk_my_textedit_clamp(state);
			state->has_preferred_x = 0;
		} break;

	case NK_KEY_TEXT_WORD_LEFT:
		if (shift_mod) {
			if (!NK_MY_TEXT_HAS_SELECTION(state))
				nk_my_textedit_prep_selection_at_cursor(state);
			state->cursor = nk_my_textedit_move_to_word_previous(state);
			state->select_end = state->cursor;
			nk_my_textedit_clamp(state);
		}
		else {
			if (NK_MY_TEXT_HAS_SELECTION(state))
				nk_my_textedit_move_to_first(state);
			else {
				state->cursor = nk_my_textedit_move_to_word_previous(state);
				nk_my_textedit_clamp(state);
			}
		} break;

	case NK_KEY_TEXT_WORD_RIGHT:
		if (shift_mod) {
			if (!NK_MY_TEXT_HAS_SELECTION(state))
				nk_my_textedit_prep_selection_at_cursor(state);
			state->cursor = nk_my_textedit_move_to_word_next(state);
			state->select_end = state->cursor;
			nk_my_textedit_clamp(state);
		}
		else {
			if (NK_MY_TEXT_HAS_SELECTION(state))
				nk_my_textedit_move_to_last(state);
			else {
				state->cursor = nk_my_textedit_move_to_word_next(state);
				nk_my_textedit_clamp(state);
			}
		} break;

	case NK_KEY_DOWN: {
		struct nk_text_find find;
		struct nk_text_edit_row row;
		int i, sel = shift_mod;

		if (state->single_line) {
			/* on windows, up&down in single-line behave like left&right */
			key = NK_KEY_RIGHT;
			goto retry;
		}

		if (sel)
			nk_my_textedit_prep_selection_at_cursor(state);
		else if (NK_MY_TEXT_HAS_SELECTION(state))
			nk_my_textedit_move_to_last(state);

		/* compute current position of cursor point */

		//TODO: Due to different glyph sizes this is not a robust solution for UTF, but should
		//work fine for ASCII
		state->cursor.x++;
		nk_my_textedit_clamp(state);

	} break;

	case NK_KEY_UP: {
		struct nk_text_find find;
		struct nk_text_edit_row row;
		int i, sel = shift_mod;

		if (state->single_line) {
			/* on windows, up&down become left&right */
			key = NK_KEY_LEFT;
			goto retry;
		}

		if (sel)
			nk_my_textedit_prep_selection_at_cursor(state);
		else if (NK_MY_TEXT_HAS_SELECTION(state))
			nk_my_textedit_move_to_first(state);

		/* compute current position of cursor point */
		state->cursor.x--;
		nk_my_textedit_clamp(state);

	} break;

	case NK_KEY_DEL:
		if (state->mode == NK_TEXT_EDIT_MODE_VIEW)
			break;
		if (NK_MY_TEXT_HAS_SELECTION(state))
			nk_my_textedit_delete_selection(state);
		else {
			if (state->cursor.y < state->buffer.lines[state->cursor.x].len) {
				nk_my_vec2i end = state->cursor;
				end.y++;
				nk_my_textedit_delete(state, state->cursor, end);
			}
		}
		state->has_preferred_x = 0;
		break;

	case NK_KEY_BACKSPACE:
		if (state->mode == NK_TEXT_EDIT_MODE_VIEW)
			break;
		if (NK_MY_TEXT_HAS_SELECTION(state))
			nk_my_textedit_delete_selection(state);
		else {

			nk_my_textedit_clamp(state);
			if (state->cursor.y > 0) {
				nk_my_vec2i start;
				start = state->cursor;
				start.y--;
				nk_my_textedit_delete(state, start, state->cursor);
				--state->cursor.y;
			}
			else if (state->cursor.y == 0 && state->cursor.x > 0) {
				nk_my_textline_concat(&state->buffer.lines[state->cursor.x - 1],
					&state->buffer.lines[state->cursor.x]);
				nk_my_textbuffer_remove_line(&state->buffer, state->cursor.x);
				state->cursor.x--;
				state->cursor.y = state->buffer.lines[state->cursor.x].numGlyphs;
			}
		}
		state->has_preferred_x = 0;
		break;

	case NK_KEY_TEXT_START:
		if (shift_mod) {
			nk_my_textedit_prep_selection_at_cursor(state);
			state->cursor.x = state->select_end.x = 0;
			state->cursor.y = state->select_end.y = 0;
			state->has_preferred_x = 0;
		}
		else {
			state->cursor.x = state->select_start.x = state->select_end.x = 0;
			state->cursor.y = state->select_start.y = state->select_end.y = 0;
			state->has_preferred_x = 0;
		}
		break;

	case NK_KEY_TEXT_END:
		if (shift_mod) {
			nk_my_textedit_prep_selection_at_cursor(state);
			state->cursor.x = state->select_end.x = state->buffer.len - 1;
			state->cursor.y = state->select_end.y = state->buffer.lines[state->cursor.x].len;
			state->has_preferred_x = 0;
		}
		else {
			state->cursor.x = state->buffer.len - 1;
			state->cursor.y = state->buffer.lines[state->cursor.x].len;
			state->select_start.x = state->select_end.x = 0;
			state->select_start.y = state->select_end.y = 0;
			state->has_preferred_x = 0;
		}
		break;

	case NK_KEY_TEXT_LINE_START: {
		if (shift_mod) {
			nk_my_textedit_clamp(state);
			nk_my_textedit_prep_selection_at_cursor(state);
			//if (state->string.len && state->cursor == state->string.len)
			//	--state->cursor;
			state->cursor.y = state->select_end.y = 0;
			state->has_preferred_x = 0;
		}
		else {
			//if (state->string.len && state->cursor == state->string.len)
			//	--state->cursor;
			nk_my_textedit_clamp(state);
			nk_my_textedit_move_to_first(state);
			state->cursor.y = 0;
			state->has_preferred_x = 0;
		}
	} break;

	case NK_KEY_TEXT_LINE_END: {
		if (shift_mod) {
			nk_my_textedit_clamp(state);
			nk_my_textedit_prep_selection_at_cursor(state);
			state->has_preferred_x = 0;
			state->cursor.y = state->buffer.lines[state->cursor.x].numGlyphs;
			state->select_end = state->cursor;
		}
		else {
			nk_my_textedit_clamp(state);
			nk_my_textedit_move_to_first(state);

			state->has_preferred_x = 0;
			state->cursor.y = state->buffer.lines[state->cursor.x].numGlyphs;
		}} break;
	}
}

NK_INTERN void
nk_my_textedit_clear_state(struct nk_my_text_edit *state, enum nk_text_edit_type type,
	nk_plugin_filter filter)
{
	/* reset the state to default */
	state->undo.undo_point = 0;
	state->undo.undo_char_point = 0;
	state->undo.redo_point = NK_TEXTEDIT_UNDOSTATECOUNT;
	state->undo.redo_char_point = NK_TEXTEDIT_UNDOCHARCOUNT;
	state->select_end.x = state->select_start.x = 0;
	state->select_end.y = state->select_start.y = 0;
	state->cursor.x = 0;
	state->cursor.y = 0;
	state->has_preferred_x = 0;
	state->preferred_x = 0;
	state->cursor_at_end_of_line = 0;
	state->initialized = 1;
	state->single_line = (unsigned char)(type == NK_TEXT_EDIT_SINGLE_LINE);
	state->mode = NK_TEXT_EDIT_MODE_VIEW;
	state->filter = filter;
	state->scrollbar = nk_vec2(0, 0);
}




NK_INTERN nk_my_vec2i
nk_my_textedit_locate_coord(struct nk_my_text_edit *edit, float x, float y,
	const struct nk_user_font *font, float row_height)
{
	struct nk_my_vec2i coords;
	coords.x = -1;
	coords.y = -1;

	coords.x = y / row_height;

	if (coords.x > edit->buffer.len - 1) {
		coords.x = edit->buffer.len - 1;
	}
	if (coords.x < 0) {
		coords.x = 0;
	}

	TextLine line = edit->buffer.lines[coords.x];

	//struct nk_text_edit_row row_layout;
	//nk_my_textedit_layout_row(&row_layout, edit, coords.x, row_height, font);
	float accumX = 0;
	int glyphLen = 0;
	nk_rune unicode;
	int numGlyphs = 0;
	for (int i = 0; i < line.len; i++) {
		glyphLen = nk_utf_decode(line.text + i, &unicode, line.len - i);
		accumX += font->width(font->userdata, font->height, line.text + i, glyphLen);
		if (accumX >= x) {
			coords.y = numGlyphs;
			break;
		}
		numGlyphs++;
	}

	if (coords.y == -1) {
		coords.y = line.numGlyphs;
	}
	return coords;
}

NK_INTERN void
nk_my_textedit_click(struct nk_my_text_edit *state, float x, float y,
	const struct nk_user_font *font, float row_height)
{
	/* API click: on mouse down, move the cursor to the clicked location,
	* and reset the selection */
	state->cursor = nk_my_textedit_locate_coord(state, x, y, font, row_height);
	state->select_start = state->cursor;
	state->select_end = state->cursor;
	state->has_preferred_x = 0;
}

NK_INTERN void
nk_my_textedit_drag(struct nk_my_text_edit *state, float x, float y,
	const struct nk_user_font *font, float row_height)
{
	/* API drag: on mouse drag, move the cursor and selection endpoint
	* to the clicked location */
	nk_my_vec2i p = nk_my_textedit_locate_coord(state, x, y, font, row_height);
	if (!NK_MY_TEXT_HAS_SELECTION(state)) {
		state->select_start = state->cursor;
	}

	state->cursor = state->select_end = p;
}

NK_API int
nk_my_textedit_paste(struct nk_my_text_edit *state, char const *ctext, int len)
{
	/* API paste: replace existing selection with passed-in text */
	int glyphs;
	const char *text = (const char *)ctext;
	if (state->mode == NK_TEXT_EDIT_MODE_VIEW) return 0;

	/* if there's a selection, the paste should delete it */
	nk_my_textedit_clamp(state);
	nk_my_textedit_delete_selection(state);

	/* try to insert the characters */
	nk_my_textedit_text(state, ctext, len);
	return 0;
}

NK_API int
nk_my_textedit_cut(struct nk_my_text_edit *state)
{
	/* API cut: delete selection */
	if (state->mode == NK_TEXT_EDIT_MODE_VIEW)
		return 0;
	if (NK_MY_TEXT_HAS_SELECTION(state)) {
		nk_my_textedit_delete_selection(state); /* implicitly clamps */
		state->has_preferred_x = 0;
		return 1;
	}
	return 0;
}

NK_API struct nk_vec2
nk_my_get_cursor_pos(struct nk_my_text_edit* edit, const struct nk_user_font* font, float row_height) {
	struct nk_vec2 cursor_pos;
	cursor_pos.y = edit->cursor.x * row_height;
	cursor_pos.x = 3;
	int i = 0;
	TextLine* line = &edit->buffer.lines[edit->cursor.x];
	int glyphLen = 0;
	float glyphWidth;
	nk_rune u;
	int numGlyphs = 0;
	while (numGlyphs < edit->cursor.y && i < line->len) {
		glyphLen = nk_utf_decode(&line->text[i], &u, line->len - i);
		NK_ASSERT(glyphLen > 0);
		glyphWidth = (float)font->width(font->userdata, font->height, &line->text[i], glyphLen);
		cursor_pos.x += glyphWidth;
		i += glyphLen;
		numGlyphs++;
	}
	return cursor_pos;
}

NK_INTERN nk_flags
nk_my_do_edit(nk_flags *state, struct nk_command_buffer *out,
	struct nk_rect bounds, nk_flags flags, nk_plugin_filter filter,
	struct nk_my_text_edit *edit, const struct nk_style_edit *style,
	struct nk_input *in, const struct nk_user_font *font)
{
	struct nk_rect area;
	nk_flags ret = 0;
	float row_height;
	char prev_state = 0;
	char is_hovered = 0;
	char select_all = 0;
	char cursor_follow = 0;
	struct nk_rect old_clip;
	struct nk_rect clip;

	NK_ASSERT(state);
	NK_ASSERT(out);
	NK_ASSERT(style);
	if (!state || !out || !style)
		return ret;

	/* visible text area calculation */
	area.x = bounds.x + style->padding.x + style->border;
	area.y = bounds.y + style->padding.y + style->border;
	area.w = bounds.w - (2.0f * style->padding.x + 2 * style->border);
	area.h = bounds.h - (2.0f * style->padding.y + 2 * style->border);
	if (flags & NK_EDIT_MULTILINE)
		area.w = NK_MAX(0, area.w - style->scrollbar_size.x);
	row_height = font->height + style->row_padding;

	/* calculate clipping rectangle */
	old_clip = out->clip;
	nk_unify(&clip, &old_clip, area.x, area.y, area.x + area.w, area.y + area.h);

	/* update edit state */
	prev_state = (char)edit->active;
	is_hovered = (char)nk_input_is_mouse_hovering_rect(in, bounds);
	if (in && in->mouse.buttons[NK_BUTTON_LEFT].clicked && in->mouse.buttons[NK_BUTTON_LEFT].down) {
		edit->active = NK_INBOX(in->mouse.pos.x, in->mouse.pos.y,
			bounds.x, bounds.y, bounds.w, bounds.h);
	}

	/* (de)activate text editor */
	if (!prev_state && edit->active) {
		const enum nk_text_edit_type type = (flags & NK_EDIT_MULTILINE) ?
			NK_TEXT_EDIT_MULTI_LINE : NK_TEXT_EDIT_SINGLE_LINE;
		nk_my_textedit_clear_state(edit, type, filter);
		if (flags & NK_EDIT_ALWAYS_INSERT_MODE)
			edit->mode = NK_TEXT_EDIT_MODE_INSERT;
		if (flags & NK_EDIT_AUTO_SELECT)
			select_all = nk_true;
		if (flags & NK_EDIT_GOTO_END_ON_ACTIVATE) {
			edit->cursor.x = edit->buffer.len;
			edit->cursor.y = edit->buffer.lines[edit->buffer.len - 1].len;
			in = 0;
		}
	}
	else if (!edit->active) edit->mode = NK_TEXT_EDIT_MODE_VIEW;
	if (flags & NK_EDIT_READ_ONLY)
		edit->mode = NK_TEXT_EDIT_MODE_VIEW;

	ret = (edit->active) ? NK_EDIT_ACTIVE : NK_EDIT_INACTIVE;
	if (prev_state != edit->active)
		ret |= (edit->active) ? NK_EDIT_ACTIVATED : NK_EDIT_DEACTIVATED;

	/* handle user input */
	if (edit->active && in)
	{
		int shift_mod = in->keyboard.keys[NK_KEY_SHIFT].down;
		const float mouse_x = (in->mouse.pos.x - area.x) + edit->scrollbar.x;
		const float mouse_y = (in->mouse.pos.y - area.y) + edit->scrollbar.y;

		/* mouse click handler */
		is_hovered = (char)nk_input_is_mouse_hovering_rect(in, area);
		if (select_all) {
			nk_my_textedit_select_all(edit);
		}
		else if (is_hovered && in->mouse.buttons[NK_BUTTON_LEFT].down &&
			in->mouse.buttons[NK_BUTTON_LEFT].clicked) {
			nk_my_textedit_click(edit, mouse_x, mouse_y, font, row_height);
		}
		else if (is_hovered && in->mouse.buttons[NK_BUTTON_LEFT].down &&
			(in->mouse.delta.x != 0.0f || in->mouse.delta.y != 0.0f)) {
			nk_my_textedit_drag(edit, mouse_x, mouse_y, font, row_height);
			cursor_follow = nk_true;
		}
		else if (is_hovered && in->mouse.buttons[NK_BUTTON_RIGHT].clicked &&
			in->mouse.buttons[NK_BUTTON_RIGHT].down) {
			nk_my_textedit_key(edit, NK_KEY_TEXT_WORD_LEFT, nk_false, font, row_height);
			nk_my_textedit_key(edit, NK_KEY_TEXT_WORD_RIGHT, nk_true, font, row_height);
			cursor_follow = nk_true;
		}

		//Scrolling
		{
			edit->scrollbar.y -= in->mouse.scroll_delta * 20;
			if (edit->scrollbar.y < 0) {
				edit->scrollbar.y = 0;
			}
			else if(edit->scrollbar.y / row_height > edit->buffer.len) {
				edit->scrollbar.y = edit->buffer.len * row_height;
			}
		}


		{int i; /* keyboard input */
		int old_mode = edit->mode;
		for (i = 0; i < NK_KEY_MAX; ++i) {
			if (i == NK_KEY_ENTER || i == NK_KEY_TAB) continue; /* special case */
			if (nk_input_is_key_pressed(in, (enum nk_keys)i)) {
				nk_my_textedit_key(edit, (enum nk_keys)i, shift_mod, font, row_height);
				cursor_follow = nk_true;
			}
		}
		if (old_mode != edit->mode) {
			in->keyboard.text_len = 0;
		}}

		/* text input */
		edit->filter = filter;
		if (in->keyboard.text_len) {
			nk_my_textedit_text(edit, in->keyboard.text, in->keyboard.text_len);
			cursor_follow = nk_true;
			in->keyboard.text_len = 0;
		}

		/* enter key handler */
		if (nk_input_is_key_pressed(in, NK_KEY_ENTER)) {
			cursor_follow = nk_true;
			if (flags & NK_EDIT_CTRL_ENTER_NEWLINE && shift_mod)
				nk_my_textedit_text(edit, "\n", 1);
			else if (flags & NK_EDIT_SIG_ENTER)
				ret |= NK_EDIT_COMMITED;
			else nk_my_textedit_text(edit, "\n", 1);
		}

		/* cut & copy handler */

		{
			int copy = nk_input_is_key_pressed(in, NK_KEY_COPY);
			int cut = nk_input_is_key_pressed(in, NK_KEY_CUT);
			if ((copy || cut) && (flags & NK_EDIT_CLIPBOARD))
			{
				int glyph_len;
				nk_rune unicode;
				const char *text;
				nk_my_textedit_sortselection(edit);

				if (edit->clip.copy)
					edit->clip.copy(edit->clip.userdata, &edit->buffer, edit->select_start, edit->select_end);
				if (cut && !(flags & NK_EDIT_READ_ONLY)) {
					nk_my_textedit_cut(edit);
					cursor_follow = nk_true;
				}
			}
		}

		/* paste handler */

		{
			int paste = nk_input_is_key_pressed(in, NK_KEY_PASTE);
			if (paste && (flags & NK_EDIT_CLIPBOARD) && edit->clip.paste) {
				edit->clip.paste(edit->clip.userdata, edit);
				cursor_follow = nk_true;
			}
		}

		/* tab handler */
		{
			int tab = nk_input_is_key_pressed(in, NK_KEY_TAB);
			if (tab) {
#ifdef NK_REPLACE_TABS_WITH_SPACES
				nk_my_textedit_text(edit, "    ", 4);
#else
				nk_my_textedit_insert_glyph(&edit->buffer.lines[edit->cursor.x], &edit->cursor, "\t", 1);
#endif
				cursor_follow = nk_true;
			}
		}
	}

	/* set widget state */
	if (edit->active)
		*state = NK_WIDGET_STATE_ACTIVE;
	else nk_widget_state_reset(state);

	if (is_hovered)
		*state |= NK_WIDGET_STATE_HOVERED;

	/* DRAW EDIT */
	{/* select background colors/images  */
		const struct nk_style_item *background;
		if (*state & NK_WIDGET_STATE_ACTIVED)
			background = &style->active;
		else if (*state & NK_WIDGET_STATE_HOVER)
			background = &style->hover;
		else background = &style->normal;

		/* draw background frame */
		if (background->type == NK_STYLE_ITEM_COLOR) {
			nk_stroke_rect(out, bounds, style->rounding, style->border, style->border_color);
			nk_fill_rect(out, bounds, style->rounding, background->data.color);
		}
		else nk_draw_image(out, bounds, &background->data.image, nk_white); }

	area.w = NK_MAX(0, area.w - style->cursor_size);
	if (edit->active)
	{
		int total_lines = 1;
		struct nk_vec2 text_size = nk_vec2(0, 0);

		/* text pointer positions */
		const char *cursor_ptr = &edit->buffer.lines[edit->cursor.x].text[edit->cursor.y];
		const char *select_begin_ptr = 0;
		const char *select_end_ptr = 0;

		/* 2D pixel positions */
		struct nk_vec2 cursor_pos = nk_vec2(0, 0);
		struct nk_vec2 selection_offset_start = nk_vec2(0, 0);
		struct nk_vec2 selection_offset_end = nk_vec2(0, 0);

		text_size.y = edit->buffer.len * row_height;
		//Find cursor pos
		cursor_pos = nk_my_get_cursor_pos(edit, font, row_height);

		{
			/* scrollbar */
			if (cursor_follow)
			{
				/* update scrollbar to follow cursor */
				if (!(flags & NK_EDIT_NO_HORIZONTAL_SCROLL)) {
					/* horizontal scroll */
					const float scroll_increment = area.w * 0.25f;
					if (cursor_pos.x < edit->scrollbar.x)
						edit->scrollbar.x = (float)(int)NK_MAX(0.0f, cursor_pos.x - scroll_increment);
					if (cursor_pos.x >= edit->scrollbar.x + area.w)
						edit->scrollbar.x = (float)(int)NK_MAX(0.0f, cursor_pos.x);
				}
				else edit->scrollbar.x = 0;

				if (flags & NK_EDIT_MULTILINE) {
					/* vertical scroll */
					if (cursor_pos.y < edit->scrollbar.y)
						edit->scrollbar.y = NK_MAX(0.0f, cursor_pos.y - row_height);
					if (cursor_pos.y >= edit->scrollbar.y + area.h)
						edit->scrollbar.y = edit->scrollbar.y + row_height;
				}
				else edit->scrollbar.y = 0;
			}

			/* scrollbar widget */
			if (flags & NK_EDIT_MULTILINE)
			{
				nk_flags ws;
				struct nk_rect scroll;
				float scroll_target;
				float scroll_offset;
				float scroll_step;
				float scroll_inc;

				scroll = area;
				scroll.x = (bounds.x + bounds.w - style->border) - style->scrollbar_size.x;
				scroll.w = style->scrollbar_size.x;

				scroll_offset = edit->scrollbar.y;
				scroll_step = scroll.h * 0.10f;
				scroll_inc = scroll.h * 0.01f;
				scroll_target = text_size.y;
				edit->scrollbar.y = nk_do_scrollbarv(&ws, out, scroll, 0,
					scroll_offset, scroll_target, scroll_step, scroll_inc,
					&style->scrollbar, in, font);
			}
		}

		/* draw text */
		{struct nk_color background_color;
		struct nk_color text_color;
		struct nk_color sel_background_color;
		struct nk_color sel_text_color;
		struct nk_color cursor_color;
		struct nk_color cursor_text_color;
		const struct nk_style_item *background;
		nk_push_scissor(out, clip);

		/* select correct colors to draw */
		if (*state & NK_WIDGET_STATE_ACTIVED) {
			background = &style->active;
			text_color = style->text_active;
			sel_text_color = style->selected_text_hover;
			sel_background_color = style->selected_hover;
			cursor_color = style->cursor_hover;
			cursor_text_color = style->cursor_text_hover;
		}
		else if (*state & NK_WIDGET_STATE_HOVER) {
			background = &style->hover;
			text_color = style->text_hover;
			sel_text_color = style->selected_text_hover;
			sel_background_color = style->selected_hover;
			cursor_text_color = style->cursor_text_hover;
			cursor_color = style->cursor_hover;
		}
		else {
			background = &style->normal;
			text_color = style->text_normal;
			sel_text_color = style->selected_text_normal;
			sel_background_color = style->selected_normal;
			cursor_color = style->cursor_normal;
			cursor_text_color = style->cursor_text_normal;
		}
		if (background->type == NK_STYLE_ITEM_IMAGE)
			background_color = nk_rgba(0, 0, 0, 0);
		else background_color = background->data.color;


		{
			int startRow = edit->scrollbar.y / row_height;
			int numRows = NK_MIN(area.h / row_height, edit->buffer.len - startRow);
			float yOffset = startRow * row_height - edit->scrollbar.y;
			struct nk_rect bounds;
			bounds.x = area.x;
			bounds.w = area.w;
			bounds.h = row_height;
			struct nk_text t;
			t.background = background_color;
			t.padding = style->padding;
			t.text = style->text_normal;
			TextLine line;
			//printf("Start(%i, %i) End(%i, %i)\n", edit->select_start.x, edit->select_start.y,
				//edit->select_end.x, edit->select_end.y);
			for (int i = 0; i < numRows; i++) {
				bounds.y = area.y + i * row_height + yOffset;
				line = edit->buffer.lines[i + startRow];

				if (NK_MY_TEXT_HAS_SELECTION(edit)) {

					//Make sure selectStart and selectEnd are in the correct order
					//Calling nk_my_textedit_sortselection would fix this, but calling it
					//during a drag select messes up the selection
					nk_my_vec2i selectStart = edit->select_start;
					nk_my_vec2i selectEnd = edit->select_end;
					if (edit->select_start.x > edit->select_end.x) {
						selectStart = edit->select_end;
						selectEnd = edit->select_start;
					}
					else if (edit->select_start.x == edit->select_end.x) {
						if (edit->select_start.y > edit->select_end.y) {
							selectStart = edit->select_end;
							selectEnd = edit->select_start;
						}
					}

					struct nk_rect tempBounds;
					tempBounds = bounds;
					if (selectStart.x == i + startRow) {
						if (selectEnd.x == i + startRow) {
							//The selected text is all on one line, so split it into three sections
							//before selected, selected, and after selected
							int startByte = nk_my_bytes_to_glyph(&line, selectStart.y);
							float width = font->width(font->userdata, font->height, line.text, startByte);
							tempBounds.w = bounds.w;
							nk_widget_colored_text(out, tempBounds, line.text, startByte, line.colors, selectStart.y, edit->colorTable, &t, NK_TEXT_ALIGN_LEFT, font);


							tempBounds.x += width;
							int midByte = nk_my_bytes_to_glyph(&line, selectEnd.y);
							width = font->width(font->userdata, font->height, &line.text[startByte], midByte - startByte);
							tempBounds.w = bounds.w + tempBounds.x - bounds.x;
							t.background = style->selected_normal;
							nk_widget_colored_text(out, tempBounds, &line.text[startByte], midByte - startByte, &line.colors[selectStart.y], selectEnd.y - selectStart.y, edit->colorTable, &t, NK_TEXT_ALIGN_LEFT, font);
							t.background = background_color;

							tempBounds.x += width;
							tempBounds.w = bounds.w + tempBounds.x - bounds.x;
							nk_widget_colored_text(out, tempBounds, &line.text[midByte], line.len - midByte, &line.colors[selectEnd.y], line.numGlyphs - selectEnd.y, edit->colorTable, &t, NK_TEXT_ALIGN_LEFT, font);
							continue;
						}
						else {
							//The selected text starts on this line
							struct nk_rect tempBounds;
							tempBounds = bounds;
							int startByte = nk_my_bytes_to_glyph(&line, selectStart.y);
							float width = font->width(font->userdata, font->height, line.text, startByte);
							tempBounds.w = bounds.w;
							nk_widget_colored_text(out, tempBounds, line.text, startByte, line.colors, selectStart.y, edit->colorTable, &t, NK_TEXT_ALIGN_LEFT, font);

							tempBounds.x += width;
							//int midByte = nk_my_bytes_to_glyph(&line, selectEnd.y);
							width = font->width(font->userdata, font->height, &line.text[startByte], line.len - startByte);
							tempBounds.w = bounds.w + tempBounds.x - bounds.x;
							t.background = style->selected_normal;
							nk_widget_colored_text(out, tempBounds, &line.text[startByte], line.len - startByte, &line.colors[selectStart.y], line.numGlyphs - selectStart.y, edit->colorTable, &t, NK_TEXT_ALIGN_LEFT, font);
							t.background = background_color;
							continue;
						}
					}
					else if (selectEnd.x == i + startRow) {
						//The selected text ends on this line
						int endByte = nk_my_bytes_to_glyph(&line, selectEnd.y);
						float width = font->width(font->userdata, font->height, line.text, endByte);
						tempBounds.w = bounds.w;
						t.background = style->selected_normal;
						nk_widget_colored_text(out, tempBounds, line.text, endByte, line.colors, selectEnd.y, edit->colorTable, &t, NK_TEXT_ALIGN_LEFT, font);
						t.background = background_color;

						tempBounds.x += width;
						tempBounds.w = bounds.w + tempBounds.x - bounds.x;
						nk_widget_colored_text(out, tempBounds, &line.text[endByte], line.len - endByte, &line.colors[selectEnd.y], line.numGlyphs - selectEnd.y, edit->colorTable, &t, NK_TEXT_ALIGN_LEFT, font);
						continue;
					}
					else if (selectStart.x < i + startRow &&
						i + startRow < selectEnd.x) {
						//The entire line is selected
						t.background = style->selected_normal;
						nk_widget_colored_text(out, bounds, line.text, line.len, line.colors, line.numGlyphs, edit->colorTable, &t, NK_TEXT_ALIGN_LEFT, font);
						t.background = background_color;
						continue;
					}
				}

				nk_widget_colored_text(out, bounds, line.text, line.len, line.colors, line.numGlyphs, edit->colorTable, &t, NK_TEXT_ALIGN_LEFT, font);
			}
		}

		/* cursor */
		if (!NK_MY_TEXT_HAS_SELECTION(edit))
		{
			struct nk_rect cursor;
			cursor.w = 1;
			cursor.h = font->height;
			cursor.x = area.x + cursor_pos.x - edit->scrollbar.x;
			cursor.y = area.y + cursor_pos.y + row_height / 2.0f - cursor.h / 2.0f;
			cursor.y -= edit->scrollbar.y;
			nk_fill_rect(out, cursor, 0, cursor_color);
		}}
	}
	else {
		/* not active so just draw text */
		const struct nk_style_item *background;
		struct nk_color background_color;
		struct nk_color text_color;
		nk_push_scissor(out, clip);
		if (*state & NK_WIDGET_STATE_ACTIVED) {
			background = &style->active;
			text_color = style->text_active;
		}
		else if (*state & NK_WIDGET_STATE_HOVER) {
			background = &style->hover;
			text_color = style->text_hover;
		}
		else {
			background = &style->normal;
			text_color = style->text_normal;
		}
		if (background->type == NK_STYLE_ITEM_IMAGE)
			background_color = nk_rgba(0, 0, 0, 0);
		else background_color = background->data.color;

		int startRow = edit->scrollbar.y / row_height;
		int numRows = NK_MIN(area.h / row_height, edit->buffer.len - startRow);

		struct nk_rect bounds;
		bounds.x = area.x;
		bounds.w = area.w;
		bounds.h = row_height;
		struct nk_text t;
		t.background = background_color;
		t.padding = style->padding;
		t.text = style->text_normal;
		TextLine line;
		for (int i = 0; i < numRows; i++) {
			bounds.y = area.y + i * row_height;
			line = edit->buffer.lines[i];
			nk_widget_colored_text(out, bounds, line.text, line.len, line.colors, line.numGlyphs, edit->colorTable, &t, NK_TEXT_ALIGN_LEFT, font);
		}
	}
	nk_push_scissor(out, old_clip);
	return ret;
}
#endif
