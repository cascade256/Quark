#pragma once
#include "NuklearAndConfig.h"
#include "Globals.h"
#include "PluginProtocol.h"

void destroyAutoCompleteData(AutoCompleteData* data);
void drawAutocompleteDialog(struct nk_vec2 pos, AutoCompleteData* data, bool* active);