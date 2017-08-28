#pragma once

#include "Array.h"

bool isWhitespace(char c);
bool isLetter(char c);
bool isNumber(char c);

int parseString(const Array<char>* line, int start);
int parseIdentifier(const Array<char>* line, int start);
int parseNumber(const Array<char>* line, int start);
int parseWhitespace(const Array<char>* line, int start);