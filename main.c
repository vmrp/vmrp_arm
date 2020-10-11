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

#include "./src/include/dsm.h"

static DSM_EXPORT_FUNCS *mythroad;

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
void panic(char *msg) {
    do {
        printf("%s\n", msg);
        while (1) {
        }
    } while (0);
}

int32 br_open(const char *filename, uint32 mode) {
    int f;
    int new_mode = 0;

    if (mode & MR_FILE_RDONLY)
        new_mode = O_RDONLY;
    if (mode & MR_FILE_WRONLY)
        new_mode = O_WRONLY;
    if (mode & MR_FILE_RDWR)
        new_mode = O_RDWR;

    //如果文件存在 带此标志会导致错误
    if ((mode & MR_FILE_CREATE) && (0 != access(filename, F_OK)))
        new_mode |= O_CREAT;

    f = open(filename, new_mode, 0777);
    if (f == -1) {
        return (int32)NULL;
    }
    int32 ret = handle2int32(f);
    printf("br_open(%s,%d) fd is: %d\n", filename, new_mode, ret);
    return ret;
}

int32 br_close(int32 f) {
    if (f == 0)
        return MR_FAILED;

    int ret = close(int32ToHandle(f));
    handleDel(f);
    if (ret != 0) {
        printf("br_close(%d) err\n", f);
        return MR_FAILED;
    }
    printf("br_close(%d) suc\n", f);
    return MR_SUCCESS;
}

int32 br_read(int32 f, void *p, uint32 l) {
    int32 fd = int32ToHandle(f);
    if (fd == -1) {
        printf("br_read(%d) err\n", f);
        return MR_FAILED;
    }
    int32 readnum = read(fd, p, (size_t)l);
    if (readnum < 0) {
        printf("br_read(%d) err\n", f);
        return MR_FAILED;
    }
    return readnum;
}

int32 br_write(int32 f, void *p, uint32 l) {
    int32 writenum = write(int32ToHandle(f), p, (size_t)l);
    if (writenum < 0) {
        printf("br_write(%d) err\n", f);
        return MR_FAILED;
    }
    return writenum;
}

int32 br_seek(int32 f, int32 pos, int method) {
    off_t ret = lseek(int32ToHandle(f), (off_t)pos, method);
    if (ret < 0) {
        printf("br_seek(%d,%d) err\n", f, pos);
        return MR_FAILED;
    }
    return MR_SUCCESS;
}

int32 br_info(const char *filename) {
    struct stat s1;
    int ret = stat(filename, &s1);

    printf("br_info(%s)\n", filename);
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

int32 br_remove(const char *filename) {
    int ret = remove(filename);
    if (ret != 0) {
        printf("br_remove(%s) err, ret=%d\n", filename, ret);
        return MR_FAILED;
    }
    printf("br_remove(%s) suc\n", filename);
    return MR_SUCCESS;
}

int32 br_rename(const char *oldname, const char *newname) {
    return rename(oldname, newname);
}

int32 br_mkDir(const char *name) {
    int ret;
    if (access(name, F_OK) == 0) {  //检测是否已存在
        goto ok;
    }
    ret = mkdir(name, S_IRWXU | S_IRWXG | S_IRWXO);
    if (ret != 0) {
        printf("br_mkDir(%s) err!\n", name);
        return MR_FAILED;
    }
ok:
    printf("br_mkDir(%s) suc!\n", name);
    return MR_SUCCESS;
}

int32 br_rmDir(const char *name) {
    int ret = rmdir(name);
    if (ret != 0) {
        printf("br_rmDir(%s) err!\n", name);
        return MR_FAILED;
    }
    printf("br_rmDir(%s) suc!\n", name);
    return MR_SUCCESS;
}

int32 br_opendir(const char *name) {
    printf("br_opendir %s\n", name);

    DIR *pDir = opendir(name);
    if (pDir != NULL) {
        return handle2int32((uint32)pDir);
    }
    return MR_FAILED;
}

char *br_readdir(int32 search_handle) {
    printf("br_readdir %d\n", search_handle);
    DIR *pDir = (DIR *)int32ToHandle(search_handle);
    struct dirent *pDt = readdir(pDir);
    if (pDt != NULL) {
        return pDt->d_name;
    }
    return NULL;
}

int32 br_closedir(int32 search_handle) {
    DIR *pDir = (DIR *)int32ToHandle(search_handle);
    closedir(pDir);
    handleDel(search_handle);
    return MR_SUCCESS;
}

int32 br_sleep(uint32 ms) {
    printf("br_sleep(%d)\n", ms);
    usleep(ms * 1000);  //注意 usleep 传的是 微秒 ，所以要 *1000
    return MR_SUCCESS;
}

int32 br_getLen(const char *filename) {
    struct stat s1;
    int ret = stat(filename, &s1);
    if (ret != 0)
        return -1;
    return s1.st_size;
}

int32 br_getDatetime(mr_datetime *datetime) {
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

    printf("br_getDatetime [%d/%d/%d %d:%d:%d]\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    return MR_SUCCESS;
}

#define DSM_MEM_SIZE (5 * 1024 * 1024)  //DSM内存大小

int32 br_mem_get(char **mem_base, uint32 *mem_len) {
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
    printf("br_mem_get base=%p len=0x%X =================\n", buffer, len);
    return MR_SUCCESS;
}

int32 br_mem_free(char *mem, uint32 mem_len) {
    printf("br_mem_free!!!!\n");
    free(mem);
    return MR_SUCCESS;
}

// void logPrint(char *level, char *tag, ...) {
//     va_list ap;
//     char *fmt;
//     va_start(ap, tag);
//     fmt = va_arg(ap, char *);
//     printf("%s[%s]: ", level, tag);
//     vprintf(fmt, ap);
//     putchar('\n');
//     va_end(ap);
// }

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

void j2n_startMrp(char *path) {
    printf("vm_loadMrp entry:%s\n", path);
    // mr_registerAPP((uint8 *)buf, (int32)len, (int32)index);
#ifdef DSM_FULL
    printf("DSM_FULL\n");
    mythroad->mr_start_dsm(path);
#else
    panic("not DSM_FULL");
    printf("mr_start_dsmC(cfunction.ext, runMrpPath);\n");
    mr_start_dsmC("cfunction.ext", runMrpPath);
#endif
}

void j2n_pause() {
    printf("mr_pauseApp\n");
    mythroad->mr_pauseApp();
}

void j2n_resume() {
    printf("mr_resumeApp\n");
    mythroad->mr_resumeApp();
}

// void j2n_stop() {
//     printf("mr_stop\n");
//     mr_stop();  //仅仅是通知调用 mrc_exit()
//     mr_exit();  //最后执行
// }

// void j2n_smsRecv(char *numStr, char *contentStr) {
//     uint8 buf[64];
//     UTF8ToGBString((uint8 *)numStr, buf, sizeof(buf));

//     uint8 buf2[1024];
//     UTF8ToGBString((uint8 *)contentStr, buf2, sizeof(buf2));

//     mr_smsIndiaction((uint8 *)buf2, strlen((char *)buf2), (uint8 *)buf, MR_ENCODE_ASCII);
// }

static SDL_TimerID timeId = 0;
static int timeLock = 0;

Uint32 th2(Uint32 interval, void *param) {
    timeLock = 1;
    mythroad->mr_timer();
    timeLock = 0;
    return 0;
}

int32 br_timerStart(uint16 t) {
    printf("br_timerStart %d\n", t);
    if (!timeId) {
        timeId = SDL_AddTimer(t, th2, NULL);
    } else {
        printf("br_timerStart ignore %d======================================\n", t);
        SDL_RemoveTimer(timeId);
        timeId = SDL_AddTimer(t, th2, NULL);
    }
    return MR_SUCCESS;
}

int32 br_timerStop() {
    printf("br_timerStop\n");
    if (timeId) {
        SDL_RemoveTimer(timeId);
        timeId = 0;
    } else {
        printf("br_timerStop ignore----------------------------------------------\n");
    }
    return MR_SUCCESS;
}

void br_log(char *msg) {
    puts(msg);
    // printf(msg);
}

void br_exit(void) {
    puts("mythroad exit.\n");
}
void br_srand(uint32 seed) {
    srand(seed);
}
int32 br_rand(void) {
    return rand();
}
////////////////////////////////////////////////////////////////////////////////////////////////

// http://wiki.libsdl.org/Tutorials
// http://lazyfoo.net/tutorials/SDL/index.php

static SDL_Renderer *renderer;

void br_drawBitmap(uint16 *data, int16 x, int16 y, uint16 w, uint16 h) {
    for (uint32 i = 0; i < w; i++) {
        for (uint32 j = 0; j < h; j++) {
            int32 xx = x + i;
            int32 yy = y + j;
            if (xx < 0 || yy < 0 || xx >= SCREEN_WIDTH || yy >= SCREEN_HEIGHT) {
                continue;
            }
            uint16 color = *(data + (xx + yy * SCREEN_WIDTH));
            SDL_SetRenderDrawColor(renderer, PIXEL565R(color), PIXEL565G(color), PIXEL565B(color), 0xFF);
            SDL_RenderDrawPoint(renderer, xx, yy);
        }
    }
    SDL_RenderPresent(renderer);
}

static void keyEvent(int16 type, SDL_Keycode code) {
    switch (code) {
        case SDLK_RETURN:
            mythroad->mr_event(type, MR_KEY_SELECT, 0);
            break;
        case SDLK_w:
        case SDLK_UP:
            mythroad->mr_event(type, MR_KEY_UP, 0);
            break;
        case SDLK_s:
        case SDLK_DOWN:
            mythroad->mr_event(type, MR_KEY_DOWN, 0);
            break;
        case SDLK_a:
        case SDLK_LEFT:
            mythroad->mr_event(type, MR_KEY_LEFT, 0);
            break;
        case SDLK_d:
        case SDLK_RIGHT:
            mythroad->mr_event(type, MR_KEY_RIGHT, 0);
            break;
        case SDLK_q:
        case SDLK_LEFTBRACKET:
            mythroad->mr_event(type, MR_KEY_SOFTLEFT, 0);
            break;
        case SDLK_e:
        case SDLK_RIGHTBRACKET:
            mythroad->mr_event(type, MR_KEY_SOFTRIGHT, 0);
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

    handleInit();

    DSM_REQUIRE_FUNCS *funcs = malloc(sizeof(DSM_REQUIRE_FUNCS));
    funcs->panic = panic;
    funcs->log = br_log;
    funcs->exit = br_exit;
    funcs->srand = br_srand;
    funcs->rand = br_rand;
    funcs->mem_get = br_mem_get;
    funcs->mem_free = br_mem_free;
    funcs->timerStart = br_timerStart;
    funcs->timerStop = br_timerStop;
    funcs->get_time_ms = get_time_ms;
    funcs->getDatetime = br_getDatetime;
    funcs->sleep = br_sleep;
    funcs->open = br_open;
    funcs->close = br_close;
    funcs->read = br_read;
    funcs->write = br_write;
    funcs->seek = br_seek;
    funcs->info = br_info;
    funcs->remove = br_remove;
    funcs->rename = br_rename;
    funcs->mkDir = br_mkDir;
    funcs->rmDir = br_rmDir;
    funcs->opendir = br_opendir;
    funcs->readdir = br_readdir;
    funcs->closedir = br_closedir;
    funcs->getLen = br_getLen;
    funcs->drawBitmap = br_drawBitmap;

    mythroad = dsm_init(funcs);

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
                        mythroad->mr_event(MR_MOUSE_MOVE, event.motion.x, event.motion.y);
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    isDown = true;
                    mythroad->mr_event(MR_MOUSE_DOWN, event.motion.x, event.motion.y);
                    break;
                case SDL_MOUSEBUTTONUP:
                    isDown = false;
                    mythroad->mr_event(MR_MOUSE_UP, event.motion.x, event.motion.y);
                    break;
            }
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
