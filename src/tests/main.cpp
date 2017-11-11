#include <cassert>
#include "../Array.h"

#define NK_IMPLEMENTATION
#include "../NuklearAndConfig.h"
#include "../NuklearTextEditor.h"
#include "texteditor.cpp"
#include "array.cpp"

int main() {
	testArrays();
	testTextEditor();
	return 0;
}