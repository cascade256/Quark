
bool doSetsOfLinesMatch(const Array<TextLine>* a, const char* b[]) {
	for (int i = 0; i < a->len; i++) {
		if (strncmp(a->data[i].text.data, b[i], a->data[i].text.len) != 0) {
			return false;
		}
	}
	return true;
}

void testTextEditor() {
	//Check text input handling
	{
		//Setup 
		nk_my_text_edit state;
		arrayInit(&state.lines);

		TextLine line;
		arrayInit(&line.colors);
		arrayInit(&line.text);
		arrayAdd(&state.lines, line);

		state.cursor.line = 0;
		state.cursor.col = 0;

		//Input some text
		{
			const char* input = "Hello\nhow are you?";
			nk_my_textedit_text(&state, input, strlen(input));

			const char* res[] = { "Hello", "how are you?" };
			assert(doSetsOfLinesMatch(&state.lines, res));
		}

		//Move the cursor to the end of "Hello" and add "Bob,"
		{
			state.cursor.line = 0;
			state.cursor.col = 5;
			const char* input = " Bob,";
			nk_my_textedit_text(&state, input, strlen(input));

			const char* res[] = { "Hello Bob,", "how are you?" };
			assert(doSetsOfLinesMatch(&state.lines, res));
		}

		//Insert a new line between the current two
		{
			state.cursor = text_cursor(0, state.lines[0].text.len);

			const char* input = "\nand Alice,";
			nk_my_textedit_text(&state, input, strlen(input));

			const char* res[] = { "Hello Bob,", "and Alice,", "how are you?" };
			assert(doSetsOfLinesMatch(&state.lines, res));
		}

		//Test deleting a text selection all on one line as shown below between the two carets
		//_____^--------^_____
		{
			state.select_start = text_cursor(0, 6);
			state.select_end = text_cursor(0, 9);
			state.cursor = state.select_start;

			const char* input = "Tim";
			nk_my_textedit_text(&state, input, strlen(input));

			const char* res[] = { "Hello Tim,", "and Alice,", "how are you?" };
			assert(doSetsOfLinesMatch(&state.lines, res));
		}

		//Test deleting a text selection 
		//_____^------------
		//-------------^
		{
			state.select_start = text_cursor(0, 5);
			state.select_end = text_cursor(1, 10);
			state.cursor = state.select_end;

			const char* input = " Eve,";
			nk_my_textedit_text(&state, input, strlen(input));

			const char* res[] = { "Hello Eve,", "how are you?" };
			assert(doSetsOfLinesMatch(&state.lines, res));
		}

		//Similar to above
		//_____^--------
		//-----^________
		{
			state.select_start = text_cursor(0, 5);
			state.select_end = text_cursor(1, 3);

			const char* input = " Bob,\n";
			nk_my_textedit_text(&state, input, strlen(input));

			state.cursor = text_cursor(1, 0);
			const char* input2 = "where";
			nk_my_textedit_text(&state, input2, strlen(input2));

			const char* res[] = { "Hello Bob,", "where are you?" };
			assert(doSetsOfLinesMatch(&state.lines, res));
		}
	}
}