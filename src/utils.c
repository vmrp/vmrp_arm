#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
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

long uptimems() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec) * 1000 + (now.tv_nsec) / 1000000;
}

const struct timespec *ms2timespec(long ms, struct timespec *in) {
    in->tv_sec = ms / 1000;
    in->tv_nsec = (ms % 1000) * 1000000;
    return in;
}