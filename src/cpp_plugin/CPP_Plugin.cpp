#include "../PluginProtocol.h"
#include <string.h>
#include <stdio.h>
#include "keywords.h"
#include "build.h"
//#include "debug.h"
#include "../parson.h"
#include "CPP_Plugin.h"
#include <stdlib.h>

#define logD(message, ...) api.logFunc(__FILE__, __LINE__, LOG_DEBUG, message, ##__VA_ARGS__)
#define logI(message, ...) api.logFunc(__FILE__, __LINE__, LOG_INFO, message, ##__VA_ARGS__)
#define logW(message, ...) api.logFunc(__FILE__, __LINE__, LOG_WARNING, message, ##__VA_ARGS__)
#define logE(message, ...) api.logFunc(__FILE__, __LINE__, LOG_ERROR, message, ##__VA_ARGS__)


extern "C" {

	Plugin_API api;
	Project project;

	int parseString(char* text, int start, int buffLen) {
		int i = start;
		while (i < buffLen && (text[i] != '"')) {
			i++;
		}
		return i + 1;
	}

	bool isLetter(char c) {
		return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
	}

	bool isNumber(char c) {
		return (c >= '0') && (c <= '9');
	}

	int parseIdentifier(char* text, int start, int buffLen) {
		int i = start;
		while (i < buffLen && (isLetter(text[i]) || isNumber(text[i]) || (text[i] == '_'))) {
			i++;
		}
		return i;
	}


	int parseNumber(char* text, int start, int buffLen) {
		int i = start;
		while (i < buffLen) {
			if (isNumber(text[i])) {
				i++;
			}
			else {
				return i;
			}
		}
		return i;
	}

	bool isWhitespace(char c) {
		return (c == ' ') || (c == '\t');
	}

	int parseWhitespace(char* text, int start, int buffLen) {
		int i = start;
		while (i < buffLen) {
			if (isWhitespace(text[i])) {
				i++;
			}
			else {
				return i;
			}
		}
		return i;
	}

	void colorize(TextBuffer* buffer, int editedLine) {
		TextLine* line = &buffer->lines[editedLine];
		int i = 0;
		while (i < line->len) {
			TokenType tok = TOK_DEFAULT;
			int tokStart = i;
			char c = line->text[i];
			if (isWhitespace(c)) {
				i = parseWhitespace(line->text, i, line->len);
			}
			else if (isNumber(c)) {
				i = parseNumber(line->text, i, line->len);
				tok = TOK_NUMBER;
			}
			else if (isLetter(c)) {
				i = parseIdentifier(line->text, i, line->len);

				/*
								for(int j = tokStart; j < i; j++) {
									if(line->text[j] == ' ') {
										putchar('!');
									}
									else {
										putchar(line->text[j]);
									}

								}
								*/
								//putchar('\n');
								//printf("NumKeywords: %i\n", (sizeof(keywords) / sizeof(char*)));
								//printf("Keyword: %s\n", keywords[1]);
								//Check to see if the identier is a keyword
				bool isKeyword = true;
				for (int j = 0; j < (sizeof(keywords) / sizeof(char*)); j++) {
					isKeyword = true;

					if (strlen(keywords[j]) != i - tokStart) {
						isKeyword = false;
						continue;
					}

					for (int k = tokStart; k < i; k++) {
						//printf("%c == %c?\n", line->text[k], keywords[j][k - tokStart]);
						if (line->text[k] != keywords[j][k - tokStart]) {
							isKeyword = false;
							break;
						}
					}
					if (isKeyword) {
						break;
					}
				}
				if (isKeyword) {
					tok = TOK_RESERVED;
					//printf("Found a keyword!\n");
				}
				else {
					tok = TOK_IDENTIFIER;
				}

			}
			else if (c == '"') {
				i = parseString(line->text, i + 1, line->len);
				tok = TOK_STRING;
			}
			else if (c == '/' && line->text[i + 1] == '/') {
				i = line->len;
				tok = TOK_COMMENT;
			}
			else {
				i++;
				tok = TOK_DEFAULT;
			}
			for (int j = tokStart; j < i; j++) {
				line->colors[j] = tok;
			}
		}
	}

	void createBuildTarget(JSON_Object* target, TargetCompileData* data) {
		//Name
		const char* name = json_object_get_string(target, "name");
		char* nameCopy = new char[strlen(name) + 1];
		strcpy(nameCopy, name);
		data->name = nameCopy;

		//Type
		const char* targetType = json_object_get_string(target, "type");
		if (strcmp(targetType, "executable") == 0) {
			data->targetType = TARGET_TYPE_EXECUTABLE;
		}
		else if (strcmp(targetType, "static library") == 0) {
			data->targetType = TARGET_TYPE_STATIC_LIB;
		}
		else if (strcmp(targetType, "shared library") == 0) {
			data->targetType = TARGET_TYPE_SHARED_LIB;
		}
		else {
			data->targetType = TARGET_TYPE_EXECUTABLE;
			logW("The type for target '%s' is not set! Using executable.", name);
		}

		//Source Files
		JSON_Array* sourceFiles = json_object_get_array(target, "sources");
		arrayInit(&data->sourceFiles);

		for (int i = 0; i < json_array_get_count(sourceFiles); i++) {
			const char* sourceFile = json_array_get_string(sourceFiles, i);
			char* srcFileCopy = new char[strlen(sourceFile) + 1];
			strcpy(srcFileCopy, sourceFile);
			arrayAdd(&data->sourceFiles, srcFileCopy);
		}

		for (int i = 0; i < data->sourceFiles.len; i++) {
			logI("%s\n", data->sourceFiles.data[i]);
		}

		//Include Dirs
		JSON_Array* includeDirsJSON = json_object_get_array(target, "include");
		arrayInit(&data->includeDirs);

		for (int i = 0; i < json_array_get_count(includeDirsJSON); i++) {
			const char* includeDir = json_array_get_string(includeDirsJSON, i);
			char* incDirCopy = new char[strlen(includeDir) + 1];
			strcpy(incDirCopy, includeDir);
			arrayAdd(&data->includeDirs, incDirCopy);
		}

		//Libs
		JSON_Array* libsJSON = json_object_get_array(target, "libs");
		arrayInit(&data->libs);

		for (int i = 0; i < json_array_get_count(libsJSON); i++) {
			const char* lib = json_array_get_string(libsJSON, i);
			char* libCopy = new char[strlen(lib) + 1];
			strcpy(libCopy, lib);
			arrayAdd(&data->libs, libCopy);
		}
	}


	void parseProjectFile() {
		arrayInit(&project.targets);
		char* buffer;

		JSON_Value* root;
		root = json_parse_file("build.json");
		JSON_Value_Type type = json_value_get_type(root);
		JSON_Object* rootObj = json_value_get_object(root);
		JSON_Array* targets = json_object_get_array(rootObj, "targets");

		for (int i = 0; i < json_array_get_count(targets); i++) {
			JSON_Value* target = json_array_get_value(targets, i);
			if (json_value_get_type(target) == JSONObject) {
				TargetCompileData targetData;
				createBuildTarget(json_value_get_object(target), &targetData);
				arrayAdd(&project.targets, targetData);
			}
			else {
				printf("Invalid build target!\n");
			}
		}

		printf("Number of Targets: %i\n", json_array_get_count(targets));
	}

	void build() {
		buildProject(project);
	}

	void run() {
		//Run the first executable target
		for (int i = 0; i < project.targets.len; i++) {
			if (project.targets.data[i].targetType == TARGET_TYPE_EXECUTABLE) {
				system(project.targets.data[i].name);
			}
		}
	}

	__declspec(dllexport) void getPluginInfo(PluginInfo* info) {
		strcpy(info->name, "Hallo");
	}

	__declspec(dllexport) void initPlugin(Plugin_API pluginAPI) {
		api = pluginAPI;
		api.registerColorizer(colorize, "cpp");
		api.registerColorizer(colorize, "c");
		api.registerColorizer(colorize, "h");

		int menuID = api.registerMenu("C/C++");
		api.registerMenuItem(menuID, "Rebuild", build);
		api.registerMenuItem(menuID, "Run", run);
		//api.registerMenuItem(menuID, "Debug", startDebugging);

		parseProjectFile();
	}

	__declspec(dllexport) void destroyPlugin(Plugin_API api) {

	}

}
