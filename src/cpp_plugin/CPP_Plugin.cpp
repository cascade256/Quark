#include "../PluginProtocol.h"
#include <string.h>
#include <stdio.h>
#include "keywords.h"
#include "build.h"
//#include "debug.h"
#include "../parson.h"
#include "../ParsingUtils.h"
#include "CPP_Plugin.h"
#include <stdlib.h>
#include "../GlobalsStruct.h"

Project project;
Global* g;

void colorize(Array<TextLine>* lines, int editedLine) {
	TextLine* line = &lines->data[editedLine];
	int i = 0;
	while (i < line->text.len) {
		Token_Type tok = TOK_DEFAULT;
		int tokStart = i;
		char c = line->text.data[i];
		if (isWhitespace(c)) {
			i = parseWhitespace(&line->text, i);
		}
		else if (isNumber(c)) {
			i = parseNumber(&line->text, i);
			tok = TOK_NUMBER;
		}
		else if (isLetter(c)) {
			i = parseIdentifier(&line->text, i);

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
					if (line->text.data[k] != keywords[j][k - tokStart]) {
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
			i = parseString(&line->text, i + 1);
			tok = TOK_STRING;
		}
		else if (c == '/' && line->text.data[i + 1] == '/') {
			i = line->text.len;
			tok = TOK_COMMENT;
		}
		else {
			i++;
			tok = TOK_DEFAULT;
		}
		for (int j = tokStart; j < i; j++) {
			line->colors.data[j] = tok;
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

	printf("Number of Targets: %i\n", (int)json_array_get_count(targets));
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

void drawMenu() {
	if (nk_menu_begin_label(g->ctx, "C/C++", NK_TEXT_LEFT, nk_vec2i(120, 200))) {
		nk_layout_row_dynamic(g->ctx, 25, 1);

		if (nk_menu_item_label(g->ctx, "Run", NK_TEXT_LEFT)) {
			addJob(run);
		}
		if (nk_menu_item_label(g->ctx, "Build", NK_TEXT_LEFT)) {
			addJob(build);
		}

		nk_menu_end(g->ctx);
	}
}

EXPORT void initPlugin(Global* globals) {
	g = globals;
	hashmap_put(g->colorizers, "cpp", (void*)colorize);
	hashmap_put(g->colorizers, "c", (void*)colorize);
	hashmap_put(g->colorizers, "h", (void*)colorize);

	arrayAdd(&g->menubarCBs, drawMenu);

	parseProjectFile();
}

EXPORT void destroyPlugin() {

}


