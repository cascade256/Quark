#include "OpenFileManager.h"


#include "Globals.h"


void appendLine(TextBuffer* buffer, const TextLine* line) {
	if (buffer->capacity == buffer->len) {
		//No room for more lines, expand the buffer!
		TextLine* oldLines = buffer->lines;
		buffer->capacity += 10;
		buffer->lines = new TextLine[buffer->capacity];
		memcpy(buffer->lines, oldLines, buffer->len * sizeof(TextLine));
		delete[] oldLines;
	}
	buffer->lines[buffer->len].text = new char[line->len];
	buffer->lines[buffer->len].capacity = line->len;
	buffer->lines[buffer->len].len = line->len;
	buffer->lines[buffer->len].colors = new char[line->numGlyphs];
	buffer->lines[buffer->len].numGlyphs = line->numGlyphs;
	buffer->lines[buffer->len].colorCapacity = line->numGlyphs;
	memcpy(buffer->lines[buffer->len].text, line->text, line->len * sizeof(char));
	memcpy(buffer->lines[buffer->len].colors, line->colors, line->numGlyphs * sizeof(char));
	buffer->len++;
}

void appendChar(TextLine* line, char c) {
	//Increase the capacity if it is too small
	if (line->capacity == line->len) {
		char* oldText = line->text;
		line->capacity = line->capacity * 2;
		line->text = new char[line->capacity];
		memcpy(line->text, oldText, line->len);
		delete[] oldText;
	}
	if (line->colorCapacity == line->numGlyphs) {
		char* oldColors = line->colors;
		line->colorCapacity = line->colorCapacity * 2;
		line->colors = new char[line->colorCapacity];
		memcpy(line->colors, oldColors, line->numGlyphs);
		delete[] oldColors;
	}
	line->text[line->len] = c;
	line->colors[line->numGlyphs] = 0;
	line->len++;
	line->numGlyphs++;
}

void openFile(OpenFiles* files, const char* path) {
	//Make sure the file is valid
	FILE* file;
	file = fopen(path, "r");
	if(!file) {
			printf("Failed to open the file: %s\n", path);
			return;
	}

	//Expand the array by 1
	MyOpenFile* oldFiles = files->openFiles;
	int numOldFiles = files->len;

	files->openFiles = new MyOpenFile[numOldFiles + 1];
	files->len = numOldFiles + 1;

	for (int i = 0; i < numOldFiles; i++) {
		files->openFiles[i] = oldFiles[i];
	}
	delete[] oldFiles;

	//Fill in the new file info
	TextBuffer buffer;
	buffer.capacity = 10;
	buffer.lines = new TextLine[buffer.capacity];
	buffer.len = 0;

	TextLine temp;
	temp.capacity = 1024;
	temp.text = new char[temp.capacity];
	temp.colorCapacity = 1024;
	temp.colors = new char[temp.colorCapacity];
	temp.numGlyphs = 0;
	temp.len = 0;

	char c = fgetc(file);
	while (c != EOF) {
		if (c == '\n') {
			appendLine(&buffer, &temp);
			temp.len = 0;
			temp.numGlyphs = 0;
		}
		else {
			appendChar(&temp, c);
		}

		c = fgetc(file);
	}
	appendLine(&buffer, &temp);

	delete[] temp.text;
	delete[] temp.colors;


	MyOpenFile openFile;
	openFile.edit.buffer = buffer;
	int pathLen = strlen(path);
	openFile.path = new char[pathLen + 1];
	strncpy(openFile.path, path, pathLen + 1);
	openFile.unsaved = false;

	const char* fileName;
#ifdef __WIN32__
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
	openFile.edit.select_end.x = openFile.edit.select_start.x = openFile.edit.cursor.x = 0;
	openFile.edit.select_end.y = openFile.edit.select_start.y = openFile.edit.cursor.y = 0;
	openFile.edit.has_preferred_x = 0;
	openFile.edit.preferred_x = 0;
	openFile.edit.cursor_at_end_of_line = 0;
	openFile.edit.initialized = 1;
	openFile.edit.single_line = 0;
	openFile.edit.mode = NK_TEXT_EDIT_MODE_INSERT;
	openFile.edit.filter = nk_filter_default;
	openFile.edit.scrollbar = nk_vec2(0, 0);
	openFile.edit.colorTable = g->theme;

	for (int i = 0; i < buffer.len; i++) {
		assert(buffer.lines[i].len >= 0);
	}

	const char* ext = &strrchr(fileName, '.')[1];
	if (ext != NULL) {
		//Setup the colorizer
		any_t value;
		if (hashmap_get(g->colorizers, ext, &value) == MAP_OK) {
			openFile.edit.colorize = (Colorize_Func)value;
			for (int i = 0; i < openFile.edit.buffer.len; i++) {
				openFile.edit.colorize(&openFile.edit.buffer, i);
			}
		}
		else {
			openFile.edit.colorize = NULL;
			printf("No colorizer found\n");
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


	files->openFiles[files->len - 1] = openFile;

	/*
	for (int i = 0; i < openFile.buffer.len; i++) {
		for (int j = 0; j < openFile.buffer.lines[i].len; j++) {
			putchar(openFile.buffer.lines[i].text[j]);
		}
		putchar('\n');
	}
	*/
}

void saveFile(MyOpenFile* file) {
	assert(false);
	return;
	FILE* f;
	f = fopen(file->path, "w");
	if (f != NULL) {
		TextBuffer buffer = file->edit.buffer;
		for (int i = 0; i < buffer.len; i++) {
			fwrite(buffer.lines[i].text, sizeof(char), buffer.lines[i].len, f);
		}
		fclose(f);
		file->unsaved = false;
	}
	else {
		printf("Couldn't open the file for writing\n");
	}
}

void destroyTextBuffer(TextBuffer* buffer) {
	for (int i = 0; i < buffer->len; i++) {
		delete[] buffer->lines[i].text;
		delete[] buffer->lines[i].colors;
	}
	delete[] buffer->lines;
}

void destroyFiles(OpenFiles* files) {
	for (int i = 0; i < files->len; i++) {
		destroyTextBuffer(&files->openFiles[i].edit.buffer);
		delete[] files->openFiles[i].path;
		delete[] files->openFiles[i].name;
	}
	delete[] files->openFiles;
}
