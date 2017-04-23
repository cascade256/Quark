#pragma once

#ifndef _WIN32
#define __stdcall
#endif

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

enum TokenType {
	TOK_DEFAULT,
	TOK_COMMENT,
	TOK_IDENTIFIER,
	TOK_RESERVED,
	TOK_NUMBER,
	TOK_STRING
};


struct TextLine {
	char* text;
	int len;
	int capacity;
	char* colors;//One color per glyph, each glyph could require multiple bytes
	int colorCapacity; //the size of the colors array
	int numGlyphs;// The number of glyphs in this line, and also the length of colors
};

struct TextBuffer {
	TextLine* lines;
	int len;
	int capacity;
};

typedef void(*Colorize_Func)(TextBuffer* buffer, int editedLine);
typedef void(*AutoComplete_Func)(AutoCompleteData* data, TextBuffer* buffer);

struct PluginInfo {
	char name[1024];
};

enum PluginAttachmentType {
	PA_TYPE_COLORIZER,
	PA_TYPE_AUTOCOMPLETE,
	PA_TYPE_TOOLBAR
};

struct PA_ColorizerData {
	Colorize_Func func;
	char* fileTypes;
};

struct PA_AutoCompleteData {
	AutoComplete_Func func;
	char* fileTypes;
};

struct PluginAttachment {
	PluginAttachmentType type;
	void* data;
};

typedef void(*RegisterColorizerFunc)(Colorize_Func, const char* fileExtension);
typedef void(*RegisterAutocompleterFunc)(AutoComplete_Func, const char* fileExtension);

struct Plugin_API {
	RegisterColorizerFunc registerColorizer;
	RegisterAutocompleterFunc registerAutocompleter;
};

typedef void(__stdcall *type_initPlugin)(Plugin_API);
typedef void(__stdcall *type_destroyPlugin)(Plugin_API);
typedef void(__stdcall *type_getPluginInfo)(PluginInfo*);

struct Plugin {
	type_getPluginInfo getPluginInfo;
	type_initPlugin initPlugin;
	type_destroyPlugin destroyPlugin;

};
