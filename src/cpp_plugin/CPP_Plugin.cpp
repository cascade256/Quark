#include "../PluginProtocol.h"
#include <string.h>
#include <stdio.h>
extern "C" {

    int parseString(char* text, int start, int buffLen) {
        int i = start;
        while(i < buffLen && (text[i] != '\"')){
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
        while(i < buffLen && (isLetter(text[i]) || isNumber(text[i]))){
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
        printf("COlorize\n");
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
                tok = TOK_IDENTIFIER;
            }
            else if(c == '\"') {
                i = parseString(line->text, i, line->len);
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

    void getPluginInfo(PluginInfo* info) {
        strcpy(info->name, "Hallo");
    }

    void initPlugin(Plugin_API api) {
      api.registerColorizer(colorize, "cpp");
      api.registerColorizer(colorize, "c");
      api.registerColorizer(colorize, "h");
    }

    void destroyPlugin(Plugin_API api) {

    }

}
