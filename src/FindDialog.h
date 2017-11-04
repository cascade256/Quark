#pragma once
#include "Array.h"
#include "NuklearAndConfig.h"
#include "OpenFileManager.h"

#define MAX_SEARCH_LEN 1024

struct TextPos {
	int line;
	int col;
};

struct FindDialogCache {
    Array<TextPos> matches;
    int currentMatch;
    bool searchIsDirty;
    char lastSearch[MAX_SEARCH_LEN];
    char currentSearch[MAX_SEARCH_LEN];
    int used;
    nk_window* popup;
};

bool drawFindDialog(MyOpenFile* file, struct nk_vec2 pos);
