#ifndef utils_H__
#define utils_H__

#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#define MAKERGB565(r, g, b) (uint16_t)(((uint32_t)(r >> 3) << 11) | ((uint32_t)(g >> 2) << 5) | ((uint32_t)(b >> 3)))
#define PIXEL565R(v) ((((uint32_t)v >> 11) << 3) & 0xff)
#define PIXEL565G(v) ((((uint32_t)v >> 5) << 2) & 0xff)
#define PIXEL565B(v) (((uint32_t)v << 3) & 0xff)


int getFileType(const char *name);
int getFileSize(const char *path);
long uptimems();
const struct timespec *ms2timespec(long ms, struct timespec *in);

#define ANDROID_LOG_INFO "<info>"
#define ANDROID_LOG_WARN "<warn>"
#define ANDROID_LOG_ERROR "<error>"
#define ANDROID_LOG_DEBUG "<debug>"

void __android_log_print(char *level, char *tag, ...);

#endif  // utils_H__
