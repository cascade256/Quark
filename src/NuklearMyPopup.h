#pragma once

#include "nuklear/nuklear.h"


NK_API int nk_my_popup_begin(struct nk_context *ctx, nk_window* popup, enum nk_popup_type type, 
	const char *title, nk_flags flags, struct nk_rect rect);
NK_API void nk_my_popup_close(struct nk_context *ctx);
NK_API void nk_my_popup_end(struct nk_context *ctx);


#ifdef NK_IMPLEMENTATION

/* --------------------------------------------------------------
*
*                         MY POPUP
*
* --------------------------------------------------------------*/
NK_API int
nk_my_popup_begin(struct nk_context *ctx, nk_window* popup, enum nk_popup_type type,
	const char *title, nk_flags flags, struct nk_rect rect)
{
	struct nk_window *win;
	struct nk_panel *panel;

	int title_len;
	nk_hash title_hash;
	nk_size allocated;

	NK_ASSERT(ctx);
	NK_ASSERT(title);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout)
		return 0;

	win = ctx->current;
	panel = win->layout;
	NK_ASSERT(!(panel->type & NK_PANEL_SET_POPUP) && "popups are not allowed to have popups");
	title_len = (int)nk_strlen(title);
	title_hash = nk_murmur_hash(title, (int)title_len, NK_PANEL_POPUP);

	if (!popup) {
		popup = (struct nk_window*)nk_create_window(ctx);
		popup->parent = win;
		win->popup.win = popup;
		win->popup.active = 0;
		win->popup.type = NK_PANEL_POPUP;
	}

	/* make sure we have to correct popup */
	if (win->popup.name != title_hash) {
		if (!win->popup.active) {
			nk_zero(popup, sizeof(*popup));
			win->popup.name = title_hash;
			win->popup.active = 1;
			win->popup.type = NK_PANEL_POPUP;
		}
		else return 0;
	}

	/* popup position is local to window */
	ctx->current = popup;
	rect.x += win->layout->clip.x;
	rect.y += win->layout->clip.y;

	/* setup popup data */
	popup->parent = win;
	popup->bounds = rect;
	popup->seq = ctx->seq;
	popup->layout = (struct nk_panel*)nk_create_panel(ctx);
	popup->flags = flags;
	popup->flags |= NK_WINDOW_BORDER;
	if (type == NK_POPUP_DYNAMIC)
		popup->flags |= NK_WINDOW_DYNAMIC;

	popup->buffer = win->buffer;
	nk_start_popup(ctx, win);
	allocated = ctx->memory.allocated;
	nk_push_scissor(&popup->buffer, nk_null_rect);

	if (nk_panel_begin(ctx, title, NK_PANEL_POPUP)) {
		/* popup is running therefore invalidate parent panels */
		/*struct nk_panel *root;
		root = win->layout;
		while (root) {
			root->flags |= NK_WINDOW_ROM;
			root->flags &= ~(nk_flags)NK_WINDOW_REMOVE_ROM;
			root = root->parent;
		}
		win->popup.active = 1;*/
		popup->layout->offset_x = &popup->scrollbar.x;
		popup->layout->offset_y = &popup->scrollbar.y;
		popup->layout->parent = win->layout;
		return 1;
	}
	else {
		/* popup was closed/is invalid so cleanup */
		struct nk_panel *root;
		root = win->layout;
		while (root) {
			root->flags |= NK_WINDOW_REMOVE_ROM;
			root = root->parent;
		}
		win->layout->popup_buffer.active = 0;
		win->popup.active = 0;
		ctx->memory.allocated = allocated;
		ctx->current = win;
		nk_free_panel(ctx, popup->layout);
		popup->layout = 0;
		return 0;
	}
}

NK_API void
nk_my_popup_close(struct nk_context *ctx, nk_window* popup)
{
	NK_ASSERT(ctx);
	if (!ctx || !ctx->current) return;

	NK_ASSERT(popup->parent);
	NK_ASSERT(popup->layout->type & NK_PANEL_SET_POPUP);
	popup->flags |= NK_WINDOW_HIDDEN;
}

NK_API void
nk_my_popup_end(struct nk_context *ctx)
{
	struct nk_window *win;
	struct nk_window *popup;

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout)
		return;

	popup = ctx->current;
	if (!popup->parent) return;
	win = popup->parent;
	if (popup->flags & NK_WINDOW_HIDDEN) {
		struct nk_panel *root;
		root = win->layout;
		while (root) {
			root->flags |= NK_WINDOW_REMOVE_ROM;
			root = root->parent;
		}
		win->popup.active = 0;
	}
	nk_push_scissor(&popup->buffer, nk_null_rect);
	nk_end(ctx);

	win->buffer = popup->buffer;
	nk_finish_popup(ctx, win);
	ctx->current = win;
	nk_push_scissor(&win->buffer, win->layout->clip);
}

#endif
