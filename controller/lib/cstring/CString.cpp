#include "CString.h"

int cstring::append(char* str, size_t bufferLen, const char* src) {
    size_t availableSpace = bufferLen - strlen(str) - 1;
    size_t srcLen = strlen(src);
    strncat(str, src, availableSpace);
    int leftover = srcLen > availableSpace ? srcLen - availableSpace : 0;
    return leftover;
}

int cstring::append(char* str, size_t bufferLen, double value, int precision) {
    size_t availableSpace = bufferLen - strlen(str) - 1;
    char* end = str + strlen(str);
    size_t written = snprintf(end, availableSpace + 1, "%.*g", precision, value);
    int leftover = written > availableSpace ? written - availableSpace : 0;
    return leftover;
}

void cstring::trim(char* str) {
    char* start = str;
    while (isspace((unsigned char)*start) || (*start == '\n')) {
        ++start;
    }

    char* end = start + strlen(start) - 1;
    while ((end > start) && (isspace((unsigned char)*end) || (*end == '\n'))) {
        --end;
    }

    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}