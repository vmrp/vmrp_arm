#include "main.h"

#include <SDL2/SDL.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "./mr/include/encode.h"


static uint16 *cacheScreenBuffer;  //缓冲屏幕地址
int SCRW = SCREEN_WIDTH;
int SCRH = SCREEN_HEIGHT;

static char runMrpPath[DSM_MAX_FILE_LEN + 1];


#define MKDIR(as) mkdir(as, S_IRWXU | S_IRWXG | S_IRWXO)
/////////////////////////////////////////////////////////////////
#define HANDLE_NUM 64

// 因为系统句柄转成int32可能是负数，导致mrp编程不规范只判断是否大于0时出现遍历文件夹为空的bug，需要有一种转换机制避免返回负数
// 0号下标不使用，下标作为mrp使用的句柄，值为系统的句柄，值为-1时表示未使用
static uint32 handles[HANDLE_NUM + 1];

static void handleInit() {
    for (int i = 1; i <= HANDLE_NUM; i++) {
        handles[i] = -1;
    }
}
// 注意： mrc_open需要返回0表示失败， mrc_findStart需要返回-1表示失败，这里没做区分
static int32 handle2int32(uint32 v) {
    for (int i = 1; i <= HANDLE_NUM; i++) {
        if (handles[i] == -1) {
            handles[i] = v;
            return i;
        }
    }
    return -1;  // 失败
}

static uint32 int32ToHandle(int32 v) {
    if (v <= 0 || v > HANDLE_NUM) {
        return -1;
    }
    return handles[v];
}

static void handleDel(int32 v) {
    if (v <= 0 || v > HANDLE_NUM) {
        return;
    }
    handles[v] = -1;
}
/////////////////////////////////////////////////////////////////
MR_FILE_HANDLE mr_open(const char *filename, uint32 mode) {
    int f;
    int new_mode = 0;
    char fullpathname[DSM_MAX_FILE_LEN] = {0};

    if (mode & MR_FILE_RDONLY)
        new_mode = O_RDONLY;
    if (mode & MR_FILE_WRONLY)
        new_mode = O_WRONLY;
    if (mode & MR_FILE_RDWR)
        new_mode = O_RDWR;

    //如果文件存在 带此标志会导致错误
    if ((mode & MR_FILE_CREATE) && (0 != access(fullpathname, F_OK)))
        new_mode |= O_CREAT;

    f = open(get_filename(fullpathname, filename), new_mode, 0777);
    if (f == -1) {
        return (MR_FILE_HANDLE)NULL;
    }
    int32 ret = handle2int32(f);
    LOGI("mr_open(%s,%d) fd is: %d", fullpathname, new_mode, ret);
    return ret;
}

int32 mr_close(MR_FILE_HANDLE f) {
    if (f == 0)
        return MR_FAILED;

    int ret = close(int32ToHandle(f));
    handleDel(f);
    if (ret != 0) {
        LOGE("mr_close(%d) err", f);
        return MR_FAILED;
    }
    LOGI("mr_close(%d) suc", f);
    return MR_SUCCESS;
}

int32 mr_read(MR_FILE_HANDLE f, void *p, uint32 l) {
    if (f != font_sky16_f) {
        LOGI("mr_read %d,%p,%d", f, p, l);
    }
    int32 fd = int32ToHandle(f);
    if (fd == -1) {
        LOGE("mr_read(%d) err", f);
        return MR_FAILED;
    }
    int32 readnum = read(fd, p, (size_t)l);
    if (readnum < 0) {
        LOGE("mr_read(%d) err", f);
        return MR_FAILED;
    }
    return readnum;
}
int32 mr_write(MR_FILE_HANDLE f, void *p, uint32 l) {
    LOGI("mr_write %d,%p,%d", f, p, l);
    int32 writenum = write(int32ToHandle(f), p, (size_t)l);
    if (writenum < 0) {
        LOGE("mr_write(%d) err", f);
        return MR_FAILED;
    }
    return writenum;
}

int32 mr_seek(MR_FILE_HANDLE f, int32 pos, int method) {
    off_t ret = lseek(int32ToHandle(f), (off_t)pos, method);
    if (ret < 0) {
        LOGE("mr_seek(%d,%d) err", f, pos);
        return MR_FAILED;
    }
    return MR_SUCCESS;
}

int32 mr_info(const char *filename) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    struct stat s1;
    int ret = stat(get_filename(fullpathname, filename), &s1);

    LOGI("mr_info(%s)", fullpathname);

    if (ret != 0) {
        return MR_IS_INVALID;
    }
    if (s1.st_mode & S_IFDIR) {
        return MR_IS_DIR;
    } else if (s1.st_mode & S_IFREG) {
        return MR_IS_FILE;
    }
    return MR_IS_INVALID;
}

int32 mr_remove(const char *filename) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    int ret;

    ret = remove(get_filename(fullpathname, filename));
    if (ret != 0) {
        LOGE("mr_remove(%s) err, ret=%d", fullpathname, ret);
        return MR_FAILED;
    }
    LOGI("mr_remove(%s) suc", fullpathname);
    return MR_SUCCESS;
}

int32 mr_mkDir(const char *name) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    int ret;

    get_filename(fullpathname, name);
    if (access(fullpathname, F_OK) == 0) {  //检测是否已存在
        goto ok;
    }

    ret = mkdir(fullpathname, S_IRWXU | S_IRWXG | S_IRWXO);
    if (ret != 0) {
        LOGE("mr_mkDir(%s) err!", fullpathname);
        return MR_FAILED;
    }
ok:
    LOGI("mr_mkDir(%s) suc!", fullpathname);
    return MR_SUCCESS;
}
int32 mr_rmDir(const char *name) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    int ret;

    get_filename(fullpathname, name);

    //删除权限
    if (strcmp2(fullpathname, "./") == 0) {
        LOGE("Has no right to delete this directory:%s ", fullpathname);
        return MR_FAILED;
    }

    if (strcmp2(fullpathname, "./mythroad/") == 0) {
        LOGE("Has no right to delete this directory:%s ", fullpathname);
        return MR_FAILED;
    }

    ret = rmdir(fullpathname);
    if (ret != 0) {
        LOGE("mr_rmDir(%s) err!", fullpathname);
        return MR_FAILED;
    }

    LOGI("mr_rmDir(%s) suc!", fullpathname);
    return MR_SUCCESS;
}

MR_FILE_HANDLE mr_findStart(const char *name, char *buffer, uint32 len) {
    if (!name || !buffer || len == 0)
        return MR_FAILED;

    DIR *pDir;
    struct dirent *pDt;
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    int32 ret;

    get_filename(fullpathname, name);

    LOGI("mr_findStart %s", fullpathname);

    if ((pDir = opendir(fullpathname)) != NULL) {
        ret = handle2int32((uint32)pDir);
        LOGI("mr_findStart readdir %d", ret);
        if ((pDt = readdir(pDir)) != NULL) {
            LOGI("mr_findStart readdir %s", pDt->d_name);
            memset2(buffer, 0, len);
            UTF8ToGBString((uint8 *)pDt->d_name, (uint8 *)buffer, len);
        } else {
            LOGW("mr_findStart: readdir FAIL!");
        }

        return ret;
    } else {
        LOGE("mr_findStart %s: opendir FAIL!", fullpathname);
    }

    return MR_FAILED;
}

int32 mr_findGetNext(MR_FILE_HANDLE search_handle, char *buffer, uint32 len) {
    if (!search_handle || search_handle == MR_FAILED || !buffer || len == 0)
        return MR_FAILED;
    LOGI("mr_findGetNext %d", search_handle);

    DIR *pDir = (DIR *)int32ToHandle(search_handle);
    struct dirent *pDt;

    memset2(buffer, 0, len);
    if ((pDt = readdir(pDir)) != NULL) {
        LOGI("mr_findGetNext %d %s", search_handle, pDt->d_name);
        UTF8ToGBString((uint8 *)pDt->d_name, (uint8 *)buffer, (int)len);
        return MR_SUCCESS;
    } else {
        LOGI("mr_findGetNext end");
    }
    return MR_FAILED;
}


int32 mr_findStop(MR_SEARCH_HANDLE search_handle) {
    if (!search_handle || search_handle == MR_FAILED)
        return MR_FAILED;

    DIR *pDir = (DIR *)int32ToHandle(search_handle);
    closedir(pDir);
    handleDel(search_handle);
    return MR_SUCCESS;
}

int32 mr_sleep(uint32 ms) {
    LOGI("mr_sleep(%d)", ms);
    usleep(ms * 1000);  //注意 usleep 传的是 微秒 ，所以要 *1000
    return MR_SUCCESS;
}

int32 mr_getLen(const char *filename) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    struct stat s1;
    int ret = stat(get_filename(fullpathname, filename), &s1);
    if (ret != 0)
        return -1;
    return s1.st_size;
}


int32 mr_getDatetime(mr_datetime *datetime) {
    if (!datetime)
        return MR_FAILED;

    time_t now;
    struct tm *t;

    time(&now);
    t = localtime(&now);

    datetime->year = t->tm_year + 1900;
    datetime->month = t->tm_mon + 1;
    datetime->day = t->tm_mday;
    datetime->hour = t->tm_hour;
    datetime->minute = t->tm_min;
    datetime->second = t->tm_sec;

    LOGI("mr_getDatetime [%d/%d/%d %d:%d:%d]", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    return MR_SUCCESS;
}

int32 mr_mem_get(char **mem_base, uint32 *mem_len) {
    char *buffer;
    int pagesize, pagecount, len = DSM_MEM_SIZE;

    pagesize = sysconf(_SC_PAGE_SIZE);
    if (pagesize == -1)
        panic("sysconf");

    pagecount = len / pagesize;
    len = pagesize * pagecount;
    buffer = memalign(pagesize, len);
    if (buffer == NULL)
        panic("memalign");

    //设置内存可执行权限
    if (mprotect(buffer, len, PROT_EXEC | PROT_WRITE | PROT_READ) == -1) {
        free(buffer);
        panic("mprotect");
    }

    *mem_base = buffer;
    *mem_len = len;
    LOGE("mr_mem_get base=%p len=%x =================", buffer, len);
    return MR_SUCCESS;
}

int32 mr_mem_free(char *mem, uint32 mem_len) {
    LOGE("mr_mem_free!!!!");
    free(mem);
    return MR_SUCCESS;
}

void logPrint(char *level, char *tag, ...) {
    va_list ap;
    char *fmt;
    va_start(ap, tag);
    fmt = va_arg(ap, char *);
    printf("%s[%s]: ", level, tag);
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
    if (-1 == gettimeofday(&tv, NULL)) {
        panic("get_time_ms() err");
    }
    return (int64)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
}

void main_init() {
    cacheScreenBuffer = (uint16 *)malloc(SCRW * SCRH * 2);

    handleInit();
    dsm_init(cacheScreenBuffer);
}

void j2n_startMrp(char *path) {
    const char *str = path;
    LOGD("vm_loadMrp entry:%s", str);
    UTF8ToGBString((uint8 *)str, (uint8 *)runMrpPath, sizeof(runMrpPath));
    // mr_registerAPP((uint8 *)buf, (int32)len, (int32)index);
#ifdef DSM_FULL
    LOGD("DSM_FULL");
    mr_start_dsm(runMrpPath);
#else
    LOGD("mr_start_dsmC(cfunction.ext, runMrpPath);");
    mr_start_dsmC("cfunction.ext", runMrpPath);
#endif
}

void j2n_pause() {
    LOGI("mr_pauseApp");
    mr_pauseApp();
}

void j2n_resume() {
    LOGI("mr_resumeApp");
    mr_resumeApp();
    emu_bitmapToscreen(cacheScreenBuffer, 0, 0, SCRW, SCRH);
}

void j2n_stop() {
    LOGI("mr_stop");
    mr_stop();  //仅仅是通知调用 mrc_exit()
    mr_exit();  //最后执行
}

void j2n_smsRecv(char *numStr, char *contentStr) {
    uint8 buf[64];
    UTF8ToGBString((uint8 *)numStr, buf, sizeof(buf));

    uint8 buf2[1024];
    UTF8ToGBString((uint8 *)contentStr, buf2, sizeof(buf2));

    mr_smsIndiaction((uint8 *)buf2, strlen((char *)buf2), (uint8 *)buf, MR_ENCODE_ASCII);
}

static SDL_TimerID timeId = 0;
static int timeLock = 0;

Uint32 th2(Uint32 interval, void *param) {
    timeLock = 1;
    mr_timer();
    timeLock = 0;
    return 0;
}

int32 emu_timerStart(uint16 t) {
    LOGI("emu_timerStart %d", t);
    if (!timeId) {
        timeId = SDL_AddTimer(t, th2, NULL);
    } else {
        LOGI("emu_timerStart ignore %d======================================", t);
        SDL_RemoveTimer(timeId);
        timeId = SDL_AddTimer(t, th2, NULL);
    }
    return MR_SUCCESS;
}

int32 emu_timerStop() {
    LOGI("emu_timerStop");
    if (timeId) {
        SDL_RemoveTimer(timeId);
        timeId = 0;
    } else {
        LOGI("emu_timerStop ignore----------------------------------------------");
    }
    return MR_SUCCESS;
}

void emu_finish() {
}

// 只支持240*320大小
void printScreen(char *filename, uint16 *buf) {
    // clang-format off
    unsigned char bmpHeader[70] =
    {
        0x42, 0x4D, 0x48, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00, 
        0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xC0, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 
        0x00, 0x00, 0x02, 0x58, 0x02, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    } ;
    // clang-format on

    int fh = mr_open(filename, MR_FILE_CREATE | MR_FILE_RDWR);

    mr_write(fh, bmpHeader, sizeof(bmpHeader));

    mr_write(fh, buf, 240 * 320 * 2);

    uint16 end = 0;
    mr_write(fh, &end, sizeof(end));

    mr_close(fh);
}

////////////////////////////////////////////////////////////////////////////////////////////////

// http://wiki.libsdl.org/Tutorials
// http://lazyfoo.net/tutorials/SDL/index.php

static SDL_Renderer *renderer;

void emu_bitmapToscreen(uint16 *data, int x, int y, int w, int h) {
    // printf("emu_bitmapToscreen=====x:%d, y:%d, w:%d, h:%d, scrw:%d, scrh:%d===\n", x, y, w, h, SCRW, SCRH);
    // printScreen("a.bmp", data);
    for (uint32 i = 0; i < w; i++) {
        for (uint32 j = 0; j < h; j++) {
            int32 xx = x + i;
            int32 yy = y + j;
            if (xx < 0 || yy < 0 || xx >= SCRW || yy >= SCRH) {
                continue;
            }
            if (data == cacheScreenBuffer) {
                uint16 color = *(cacheScreenBuffer + (xx + yy * SCRW));
                SDL_SetRenderDrawColor(renderer, PIXEL565R(color), PIXEL565G(color), PIXEL565B(color), 0xFF);
                SDL_RenderDrawPoint(renderer, xx, yy);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

static void keyEvent(int16 type, SDL_Keycode code) {
    switch (code) {
        case SDLK_RETURN:
            mr_event(type, MR_KEY_SELECT, 0);
            break;
        case SDLK_w:
        case SDLK_UP:
            mr_event(type, MR_KEY_UP, 0);
            break;
        case SDLK_s:
        case SDLK_DOWN:
            mr_event(type, MR_KEY_DOWN, 0);
            break;
        case SDLK_a:
        case SDLK_LEFT:
            mr_event(type, MR_KEY_LEFT, 0);
            break;
        case SDLK_d:
        case SDLK_RIGHT:
            mr_event(type, MR_KEY_RIGHT, 0);
            break;
        case SDLK_q:
        case SDLK_LEFTBRACKET:
            mr_event(type, MR_KEY_SOFTLEFT, 0);
            break;
        case SDLK_e:
        case SDLK_RIGHTBRACKET:
            mr_event(type, MR_KEY_SOFTRIGHT, 0);
            break;
        default:
            printf("key:%d\n", code);
            break;
    }
}

int main(int argc, char *args[]) {
#ifdef __x86_64__
    printf("__x86_64__\n");
#elif __i386__
    printf("__i386__\n");
#endif
    main_init();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("vmrp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    // renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);  // windows xp
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return -1;
    }

    // SDL_Surface *screenSurface = SDL_GetWindowSurface(window);
    // SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 0));
    // SDL_UpdateWindowSurface(window);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    if (argc == 1) {
        j2n_startMrp("dsm_gm.mrp");
    } else {
        j2n_startMrp(args[1]);
    }

    SDL_Event event;
    bool isLoop = true;
    bool isDown = false;
    while (isLoop) {
        while (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isLoop = false;
                break;
            }
            if (timeLock) continue;
            switch (event.type) {
                case SDL_KEYDOWN:
                    keyEvent(MR_KEY_PRESS, event.key.keysym.sym);
                    break;
                case SDL_KEYUP:
                    keyEvent(MR_KEY_RELEASE, event.key.keysym.sym);
                    break;
                case SDL_MOUSEMOTION:
                    if (isDown) {
                        mr_event(MR_MOUSE_MOVE, event.motion.x, event.motion.y);
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    isDown = true;
                    mr_event(MR_MOUSE_DOWN, event.motion.x, event.motion.y);
                    break;
                case SDL_MOUSEBUTTONUP:
                    isDown = false;
                    mr_event(MR_MOUSE_UP, event.motion.x, event.motion.y);
                    break;
            }
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
