#ifndef __NANOTTER_H__
#define __NANOTTER_H__

#include <string.h>
#include <stdlib.h>

#define VERSION_NANOTTER "nanotter alpha-0.1"

char *tuiFloatMessageBox(const char *title, const char *text, int reqinput);
char *tuiFullMessageBox(const char *title, const char *text, int reqinput);

void strWriteStringColorTimeLine(int f, const char *s);
void strWriteDataTimeLine(const char *str);

void apiGetTokenKey(void);
int apiStreamingCallback(char* ptr, size_t size, size_t nmemb, void* data);

#endif
