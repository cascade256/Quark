#include "../GlobalsStruct.h"
#include "../ParsingUtils.h"
#include "../QuarkCore.h"
#include "keywords.h"
#include <cstring>
#include <stdlib.h>

Global* g;

void run() {
	if (g->files.openFiles == NULL || g->files.len == 0) {
		logI("No open files!\n");
		return;
	}
	char* path = g->files.openFiles[g->activeFileIndex].path;
	if (strcmp(&strrchr(path, '.')[1], "py") != 0) {
		logI("Not a python file!\n");
		return;
	}

	logI("Python file to run: %s\n", path);
#ifdef _WIN32
	char cmd[1024];
	snprintf(cmd, 1024, "start cmd /K \"python %s\"", path);

	system(cmd);
#elif __linux__
	const char* tempTemplate = "QuarkRunPythonScript_XXXXXX";
	char* tempName = new char[strlen(tempTemplate) + 1];
	strcpy(tempName, tempTemplate);
	int file = mkstemp(tempName);//The script file is self deleting

	if (file < 1) {
		logE("Failed to create temp file for script, cannot launch python!\n");
		return;
	}

	const char* scriptTemplate = "python %s; read -p \"Press any key to exit\" -n1; echo; rm -- \"$0\"";
	int res = dprintf(file, scriptTemplate, "test.py");

	if (res < 1) {
		logE("Failed to write to the temp script, cannot launch python!\n");
		return;
	}
	fsync(file);

	const char* commandTemplate = "x-terminal-emulator -e \"bash %s\"";
	char buff[2048];
	memset(buff, 0, sizeof(buff));
	sprintf(buff, commandTemplate, tempName);
	logD("Command: %s\n", buff);
	system(buff);

	delete[] tempName;
#endif
}

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

			bool isKeyword = true;
			for (int j = 0; j < (sizeof(keywords) / sizeof(char*)); j++) {
				isKeyword = true;

				if (strlen(keywords[j]) != i - tokStart) {
					isKeyword = false;
					continue;
				}

				for (int k = tokStart; k < i; k++) {
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
			}
			else {
				tok = TOK_IDENTIFIER;
			}
		}
		else if (c == '"') {
			i = parseString(&line->text, i + 1);
			tok = TOK_STRING;
		}
		else if (c == '#') {
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

void drawMenu() {
	if (menuItemBegin("Python", 100)) {
		nk_layout_row_dynamic(g->ctx, 25, 1);

		if (nk_menu_item_label(g->ctx, "Run", NK_TEXT_LEFT)) {
			addJob(run);

		}
		nk_menu_end(g->ctx);
	}
}

EXPORT void initPlugin(Global* globals) {
	g = globals;
	hashmap_put(g->colorizers, "py", (void*)colorize);

	arrayAdd(&g->menubarCBs, drawMenu);
}
EXPORT void destroyPlugin() {

}

