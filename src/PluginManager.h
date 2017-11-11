#pragma once
#include "AutocompleteDialog.h"
#include "Array.h"

typedef void(*Colorize_Func)(Array<TextLine>* buffer, int editedLine);
typedef void(*AutoComplete_Func)(AutoCompleteData* data, Array<TextLine>* buffer);

void loadPlugins();
void destroyPlugins();
