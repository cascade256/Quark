#include "ParsingUtils.h"

int parseString(const Array<char>* line, int start) {
	int i = start;
	while (i < line->len && (line->data[i] != '"')) {
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

int parseIdentifier(const Array<char>* line, int start) {
	int i = start;
	while (i < line->len && (isLetter(line->data[i]) || isNumber(line->data[i]) || (line->data[i] == '_'))) {
		i++;
	}
	return i;
}


int parseNumber(const Array<char>* line, int start) {
	int i = start;
	while (i < line->len) {
		if (isNumber(line->data[i])) {
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

int parseWhitespace(const Array<char>* line, int start) {
	int i = start;
	while (i < line->len) {
		if (isWhitespace(line->data[i])) {
			i++;
		}
		else {
			return i;
		}
	}
	return i;
}