#pragma once
//#include "NuklearAndConfig.h"
#include "nuklear\nuklear.h"

struct AutoCompleteOption {
	char* title;
	char* desc;
	char* complete;
};

struct AutoCompleteData {
	int selected;
	AutoCompleteOption* options;
	int numOptions;
	int line;
	int col;
};

void destroyAutoCompleteData(AutoCompleteData* data);
void drawAutocompleteDialog(struct nk_vec2 pos, AutoCompleteData* data, bool* active);