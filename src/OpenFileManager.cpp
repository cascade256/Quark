#include "OpenFileManager.h"
#include "Globals.h"

#define NEWLINE '\n'

void appendLine(Array<TextLine>* lines, const TextLine* line) {
	TextLine copy;
	copy.text = arrayCopy(&line->text);
	copy.colors = arrayCopy(&line->colors);
	arrayAdd(lines, copy);
}

void jobbedOpenFile(void* path) {
	openFile((const char*)path);
}

void openFile(const char* path) {
	//Make sure the file is valid
	FILE* file;
	file = fopen(path, "r");
	if(!file) {
		logW("Failed to open the file: %s\n", path);
		return;
	}

	//Fill in the new file info
	Array<TextLine> lines;
	arrayInit(&lines);

	TextLine temp;
	arrayInit(&temp.text);
	arrayInit(&temp.colors);

	char c = fgetc(file);
	while (c != EOF) {
		if (c == '\n') {
			appendLine(&lines, &temp);
			temp.text.len = 0;
			temp.colors.len = 0;
		}
		else if (c == '\r') {
			appendLine(&lines, &temp);
			temp.text.len = 0;
			temp.colors.len = 0;

			//If it is followed by a '\n', skip over it so there is not double newlines
			//Otherwise put the char back into the stream
			char possibleNewline = fgetc(file);
			if (possibleNewline != '\n') {
				ungetc(possibleNewline, file);
			}
		}
		else {
			arrayAdd(&temp.text, c);
			arrayAdd(&temp.colors, (char)TOK_DEFAULT);
		}

		c = fgetc(file);
	}
	appendLine(&lines, &temp);

	for (int i = 0; i < lines.len; i++) {
		assert(lines[i].colors.len == lines[i].text.len);
	}

	delete[] temp.text.data;
	delete[] temp.colors.data;


	MyOpenFile openFile;
	openFile.edit.lines = lines;
	int pathLen = strlen(path);
	openFile.path = new char[pathLen + 1];
	strncpy(openFile.path, path, pathLen + 1);
	openFile.unsaved = false;
	arrayInit(&openFile.breakpoints);

	const char* fileName;
#ifdef _WIN32
	fileName = &strrchr(path, '\\')[1];
#else
	fileName = &strrchr(path, '/')[1];
#endif
	int nameLen = strlen(fileName);
	//Add two extra chars, a spot for a * to mark an unsaved file and the NULL byte
	openFile.name = new char[nameLen + 2];
	strncpy(openFile.name, fileName, nameLen + 1);
	openFile.name[nameLen] = '\0';
	openFile.name[nameLen + 1] = '\0';
	openFile.nameLen = nameLen + 1;
	assert(openFile.name);
	openFile.edit.undo.undo_point = 0;
	openFile.edit.undo.undo_char_point = 0;
	openFile.edit.undo.redo_point = NK_TEXTEDIT_UNDOSTATECOUNT;
	openFile.edit.undo.redo_char_point = NK_TEXTEDIT_UNDOCHARCOUNT;
	openFile.edit.select_end.line = openFile.edit.select_start.line = openFile.edit.cursor.line = 0;
	openFile.edit.select_end.col = openFile.edit.select_start.col = openFile.edit.cursor.col = 0;
	openFile.edit.has_preferred_x = 0;
	openFile.edit.preferred_x = 0;
	openFile.edit.cursor_at_end_of_line = 0;
	openFile.edit.initialized = 1;
	openFile.edit.single_line = 0;
	openFile.edit.mode = NK_TEXT_EDIT_MODE_INSERT;
	openFile.edit.filter = nk_filter_default;
	openFile.edit.scrollbar = nk_vec2(0, 0);
	openFile.edit.colorTable = g->theme.codeColors;

	for (int i = 0; i < lines.len; i++) {
		assert(lines.data[i].text.len >= 0);
	}

	const char* ext = &strrchr(fileName, '.')[1];
	if (ext != NULL) {
		//Setup the colorizer
		any_t value;
		if (hashmap_get(g->colorizers, ext, &value) == MAP_OK) {
			openFile.edit.colorize = (Colorize_Func)value;
			for (int i = 0; i < openFile.edit.lines.len; i++) {
				openFile.edit.colorize(&openFile.edit.lines, i);
			}
		}
		else {
			openFile.edit.colorize = NULL;
			logW("No colorizer found\n");
		}

		//Setup the autocompleter
		if (hashmap_get(g->autocompleters, ext, &value) == MAP_OK) {
			openFile.edit.autocomplete = (AutoComplete_Func)value;
			openFile.edit.acActive = false;
			AutoCompleteData* ac = new AutoCompleteData();
			ac->numOptions = 0;
			ac->options = NULL;
			ac->line = 0;
			ac->col = 0;
			ac->selected = 0;
			openFile.edit.acData = ac;
		}
		else {
			openFile.edit.autocomplete = NULL;
			openFile.edit.acActive = false;
		}
	}

	//Insert the new file into the openFiles array
	mtx_lock(&g->filesMutex);
	//Expand the array by 1
	MyOpenFile* oldFiles = g->files.openFiles;
	int numOldFiles = g->files.len;

	g->files.openFiles = new MyOpenFile[numOldFiles + 1];
	g->files.len = numOldFiles + 1;

	for (int i = 0; i < numOldFiles; i++) {
		g->files.openFiles[i] = oldFiles[i];
	}
	delete[] oldFiles;

	g->files.openFiles[g->files.len - 1] = openFile;
	mtx_unlock(&g->filesMutex);

}

void saveFile(MyOpenFile* file) {
	Array<TextLine>* lines = &file->edit.lines;
	FILE* f;
	f = fopen(file->path, "w");
	if (f != NULL) {
		Array<TextLine> lines = file->edit.lines;
		for (int i = 0; i < lines.len; i++) {
			fwrite(lines.data[i].text.data, sizeof(char), lines.data[i].text.len, f);
			putc(NEWLINE, f);
		}
		fclose(f);
		file->unsaved = false;
	}
	else {
		logW("Couldn't open the file for writing\n");
	}
}

void destroyTextBuffer(Array<TextLine>* lines) {
	for (int i = 0; i < lines->len; i++) {
		delete[] lines->data[i].text.data;
		delete[] lines->data[i].colors.data;
	}
	delete[] lines->data;
}

void destroyFiles(OpenFiles* files) {
	for (int i = 0; i < files->len; i++) {
		destroyTextBuffer(&files->openFiles[i].edit.lines);
		delete[] files->openFiles[i].path;
		delete[] files->openFiles[i].name;
	}
	delete[] files->openFiles;
}
