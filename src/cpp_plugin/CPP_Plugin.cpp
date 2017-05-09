#include "../PluginProtocol.h"
#include <string.h>
#include <stdio.h>
#include "keywords.h"

extern "C" {

    int parseString(char* text, int start, int buffLen) {
        int i = start;
        while(i < buffLen && (text[i] != '"')){
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
        while(i < buffLen && (isLetter(text[i]) || isNumber(text[i]) || (text[i] == '_'))){
                i++;
        }
        return i;
    }


    int parseNumber(char* text, int start, int buffLen) {
        int i = start;
        while(i < buffLen) {
            if(isNumber(text[i])) {
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
        while(i < buffLen) {
            if(isWhitespace(text[i])) {
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
        while(i < line->len) {
            TokenType tok = TOK_DEFAULT;
            int tokStart = i;
            char c = line->text[i];
            if(isWhitespace(c)) {
                i = parseWhitespace(line->text, i, line->len);
            }
            else if(isNumber(c)) {
                i = parseNumber(line->text, i, line->len);
                tok = TOK_NUMBER;
            }
            else if(isLetter(c)) {
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
                for(int j = 0; j < (sizeof(keywords) / sizeof(char*)); j++) {
                    isKeyword = true;

                    if(strlen(keywords[j]) != i - tokStart) {
                        isKeyword = false;
                        continue;
                    }

                    for(int k = tokStart; k < i; k++) {
                        //printf("%c == %c?\n", line->text[k], keywords[j][k - tokStart]);
                        if(line->text[k] != keywords[j][k - tokStart]){
                            isKeyword = false;
                            break;
                        }
                    }
                    if(isKeyword) {
                        break;
                    }
                }
                if(isKeyword) {
                    tok = TOK_RESERVED;
                    //printf("Found a keyword!\n");
                }
                else {
                    tok = TOK_IDENTIFIER;
                }

            }
            else if(c == '"') {
                i = parseString(line->text, i + 1, line->len);
                tok = TOK_STRING;
            }
            else if(c == '/' && line->text[i + 1] == '/') {
                i = line->len;
                tok = TOK_COMMENT;
            }
            else {
                i++;
                tok = TOK_DEFAULT;
            }
            for(int j = tokStart; j < i; j++) {
                line->colors[j] = tok;
            }
        }
    }

    __declspec(dllexport) void getPluginInfo(PluginInfo* info) {
        strcpy(info->name, "Hallo");
    }

    __declspec(dllexport) void initPlugin(Plugin_API api) {
      api.registerColorizer(colorize, "cpp");
      api.registerColorizer(colorize, "c");
      api.registerColorizer(colorize, "h");
    }

    __declspec(dllexport) void destroyPlugin(Plugin_API api) {

    }

}
