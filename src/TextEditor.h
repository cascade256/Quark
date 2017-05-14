#pragma once
#include "NuklearAndConfig.h"
#include "OpenFileManager.h"
#include "AutocompleteDialog.h"
#include "FindDialog.h"

void drawTextEditor(nk_context* ctx, OpenFiles* files);
void saveActiveFile(OpenFiles* files);
void showSearchDialog();
