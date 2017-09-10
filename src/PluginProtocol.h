#pragma once
#include "Array.h"

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#elif __linux__
    #define EXPORT 
#endif

enum LogLevel {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR
};

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

enum Token_Type {//The underscore is needed due to a name conflict with a Windows type
	TOK_DEFAULT,
	TOK_COMMENT,
	TOK_IDENTIFIER,
	TOK_RESERVED,
	TOK_NUMBER,
	TOK_STRING
};


struct TextLine {
	Array<char> text;
	Array<char> colors;//One color per glyph, each glyph could require multiple bytes
};

typedef void(*Colorize_Func)(Array<TextLine>* buffer, int editedLine);
typedef void(*AutoComplete_Func)(AutoCompleteData* data, Array<TextLine>* buffer);

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

struct Global;
typedef void(*Func)();

typedef void(*RegisterColorizerFunc)(Colorize_Func, const char* fileExtension);
typedef void(*RegisterAutocompleterFunc)(AutoComplete_Func, const char* fileExtension);
typedef Global*(*GetGlobalDataFunc)(void);
typedef int(*RegisterMenuFunc)(const char* name);
typedef void(*RegisterMenuItemFunc)(int menuID, const char* name, Func onClick);
typedef void(*LogFormatStringFunc)(const char* file, int line, LogLevel level, const char* formatStr, ...);


struct Plugin_API {
	RegisterColorizerFunc registerColorizer;
	RegisterAutocompleterFunc registerAutocompleter;
	GetGlobalDataFunc getGlobalData;
	RegisterMenuFunc registerMenu;
	RegisterMenuItemFunc registerMenuItem;
	LogFormatStringFunc logFunc;
};

typedef void(*type_initPlugin)(Plugin_API);
typedef void(*type_destroyPlugin)(Plugin_API);
typedef void(*type_getPluginInfo)(PluginInfo*);

struct Plugin {
	type_getPluginInfo getPluginInfo;
	type_initPlugin initPlugin;
	type_destroyPlugin destroyPlugin;
};
