#include "main.h"

#include <SDL2/SDL.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "./mr/include/encode.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

static uint16 *cacheScreenBuffer;  //缓冲屏幕地址
int SCRW = SCREEN_WIDTH;
int SCRH = SCREEN_HEIGHT;

static char runMrpPath[DSM_MAX_FILE_LEN + 1];

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
