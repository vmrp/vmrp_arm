#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "main.h"
#include "mr_helper.h"

void __android_log_print(char *level, char *tag, ...) {
    va_list ap;
    char *fmt;
    va_start(ap, tag);
    fmt = va_arg(ap, char *);
    printf("%s<%s>: ", level, tag);
    vprintf(fmt, ap);
    putchar('\n');
    va_end(ap);
}

int getFileType(const char *name) {
    struct stat s1;
    int ret;

    //返回 0 成功
    ret = stat(name, &s1);
    if (ret != 0) {
        LOGE("getFileType errno=%d", errno);
        return MR_IS_INVALID;
    }

    if (s1.st_mode & S_IFDIR)
        return MR_IS_DIR;
    else if (s1.st_mode & S_IFREG)
        return MR_IS_FILE;
    else
        return MR_IS_INVALID;
}

int getFileSize(const char *path) {
    struct stat s1;
    int ret;

    ret = stat(path, &s1);
    if (ret != 0) {
        LOGE("getFileSize errno=%d", errno);
        return -1;
    }

    return s1.st_size;
}

int64 get_uptime_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64)ts.tv_sec * 1000 + (ts.tv_nsec / 1000000);
}

int64 get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
}