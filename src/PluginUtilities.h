#include "PluginProtocol.h"
#include <string.h>

char* bufferToString(TextBuffer* buffer, unsigned long* retLen = 0);

#ifdef IMPLEMENT_PLUGIN_UTILITIES
char* bufferToString(TextBuffer* buffer, unsigned long* retLen) {
	int len = 0;
	for (int i = 0; i < buffer->len; i++) {
		len += buffer->lines[i].len + 1;
	}

	int pos = 0;
	char* str = new char[len + 1];
	for (int i = 0; i < buffer->len; i++) {
		memcpy_s(&str[pos], len - pos, buffer->lines[i].text, buffer->lines[i].len);
		pos += buffer->lines[i].len;
		str[pos] = '\n';
		pos++;
	}
	str[len] = '\0';
	*retLen = len;
	//printf(str);
	return str;
}
#endif