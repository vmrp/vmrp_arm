#include "main.h"

#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "encode.h"
#include "font_sky16_2.h"
#include "utils.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

//---------------------------------
T_EMUENV gEmuEnv;  //API LOG 控制

uint16 *cacheScreenBuffer;  //缓冲屏幕地址
int SCNW = SCREEN_WIDTH;
int SCNH = SCREEN_HEIGHT;
int showApiLog = TRUE;

static char runMrpPath[DSM_MAX_FILE_LEN + 1];

//初始化模拟器  唯一实例
void j2n_create() {
    gEmuEnv.showFile = TRUE;
    gEmuEnv.showNet = TRUE;
    gEmuEnv.showMrPlat = TRUE;
    gEmuEnv.dsmStartTime = get_time_ms();

    screenBuf = cacheScreenBuffer = (uint16 *)malloc(SCNW * SCNH * 2);

    //	tsf_init();
#ifndef DSM_FULL
    mythroad_init();
#endif
    xl_font_sky16_init();
}

void j2n_startMrp(char *path) {
    const char *str = path;
    LOGD("vm_loadMrp entry:%s", str);
    UTF8ToGBString((uint8 *)str, (uint8 *)runMrpPath, sizeof(runMrpPath));
    showApiLog = 1;
    dsm_init();
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
    LOGD("native pause!");
    LOGI("mr_pauseApp");
    mr_pauseApp();
}

void j2n_resume() {
    LOGD("native resume!");
    LOGI("mr_resumeApp");
    mr_resumeApp();
    emu_bitmapToscreen(cacheScreenBuffer, 0, 0, screenW, screenH);
}

void j2n_stop() {
    LOGD("native stop!");
    LOGI("mr_stop");
    //仅仅是通知调用 mrc_exit()
    mr_stop();
    //最后执行
    mr_exit();
}

void j2n_smsRecv(char *numStr, char *contentStr) {
    uint8 buf[64];
    UTF8ToGBString((uint8 *)numStr, buf, sizeof(buf));

    uint8 buf2[1024];
    UTF8ToGBString((uint8 *)contentStr, buf2, sizeof(buf2));

    mr_smsIndiaction((uint8 *)buf2, strlen((char *)buf2), (uint8 *)buf, MR_ENCODE_ASCII);
}

void j2n_destroy() {
    //	tsf_dispose();
    xl_font_sky16_close();
    LOGI("native destroy");
}

void j2n_callback_play_music() {
    // dsmMediaPlay.cb(param);
}

void j2n_callback_timer_out() {
    mr_timer();
}

void j2n_callback_gethostbyname() {
    // LOGI("getHost callback ip:%p", (void*)param);
    // ((MR_GET_HOST_CB)mr_soc.callBack)(param);
}

// void j2n_setStringOptions() {
//     SetDsmSDPath(str2);
//     //mythroad 路径
//     SetDsmPath(str2);
//     strncpy(dsmFactory, str2, 7);
//     strncpy(dsmType, str2, 7);
//     strncpy(dsmSmsCenter, str2, sizeof(dsmSmsCenter));
// }

void j2n_getMemoryInfo() {
    uint32 len, left, top;
    mr_getMemoryInfo(&len, &left, &top);
    printf("len:%d, left:%d, top:%d\n", len, left, top);
}

int32 emu_timerStart(uint16 t) {
    // todo
    LOGI("emu_timerStart %d", t);
    return MR_SUCCESS;
}

int32 emu_timerStop() {
    // todo
    LOGI("emu_timerStop");
    return MR_SUCCESS;
}

int32 emu_showEdit(const char *title, const char *text, int type, int max_size) {
    // todo
    return MR_FAILED;
}

/**
 * 返回编辑框内容的起始地址(编辑框的内容指针，大端unicode编码)，如果失败返回NULL.
 */
const char *emu_getEditInputContent(int32 editHd) {
    // todo
    return NULL;
}

void emu_releaseEdit(int32 editHd) {
    // todo
}

void emu_finish() {
}

void N2J_readTsfFont(uint8 **outbuf, int32 *outlen) {
    // todo
    *outbuf = NULL;
    *outlen = 0;
}

void emu_getImageSize(const char *path, int *w, int *h) {
    // todo
    *w = 0;
    *h = 0;
}

void emu_drawImage(const char *path, int x, int y, int w, int h) {
}

void emu_palySound(const char *path, int loop) {
}

void emu_stopSound(int id) {
}

void emu_musicLoadFile(const char *path) {
}

int emu_musicCMD(int cmd, int arg0, int arg1) {
    return 0;
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define MOUSE_DOWN 2
#define MOUSE_UP 3
#define MOUSE_MOVE 12
// 五笔输入法

// http://wiki.libsdl.org/Tutorials
// http://lazyfoo.net/tutorials/SDL/index.php

static SDL_Renderer *renderer;

void emu_bitmapToscreen(uint16 *data, int x, int y, int w, int h) {
    printf("emu_bitmapToscreen=============x:%d, y:%d, w:%d, h:%d, scnw:%d, scnh%d============\n", x, y, w, h, SCNW, SCNH);
    // printScreen("a.bmp", data);
    for (uint32 i = 0; i < w; i++) {
        for (uint32 j = 0; j < h; j++) {
            int32 xx = x + i;
            int32 yy = y + j;
            if (xx < 0 || yy < 0 || xx >= SCNW || yy >= SCNH) {
                continue;
            }
            if (data == cacheScreenBuffer) {
                uint16 color = *(cacheScreenBuffer + (xx + yy * SCNW));
                SDL_SetRenderDrawColor(renderer, PIXEL565R(color), PIXEL565G(color), PIXEL565B(color), 0xFF);
                // SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xFF);
                // SDL_RenderDrawPoint(renderer, 1, 1);
                SDL_RenderDrawPoint(renderer, xx, yy);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

static void eventFunc(int code, int p0, int p1) {
    // guiSetPixel(p0,p1);
    LOGI("mr_event(%d, %d, %d)", code, p0, p1);

    // if (code == MR_SMS_GET_SC) {  //获取短信中心
    //     p0 = (int)dsmSmsCenter;
    //     p1 = 0;
    // } else if (code == MR_EMU_ON_TIMER) {
    //     LOGI("call mr_timer");
    //     mr_timer();
    //     return;
    // }
    switch (code) {
        case MOUSE_DOWN:
            // printf("MOUSE_DOWN x:%d y:%d\n", p1, p2);
            mr_event(MR_MOUSE_DOWN, p0, p1);
            break;
        case MOUSE_UP:
            // printf("MOUSE_UP x:%d y:%d\n", p1, p2);
            mr_event(MR_MOUSE_UP, p0, p1);
            break;
        case MOUSE_MOVE:
            // printf("MOUSE_MOVE x:%d y:%d\n", p1, p2);
            mr_event(MR_MOUSE_MOVE, p0, p1);
            break;
    }
}

int main(int argc, char *args[]) {
#ifdef __x86_64__
    printf("__x86_64__\n");
#elif __i386__
    printf("__i386__\n");
#endif
    j2n_create();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
        j2n_startMrp("asm.mrp");
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
            } else if (event.type == SDL_KEYUP) {
                printf("key:%d\n", event.key.keysym.sym);
            } else if (event.type == SDL_MOUSEMOTION) {
                if (isDown) {
                    eventFunc(MOUSE_MOVE, event.motion.x, event.motion.y);
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                isDown = true;
                eventFunc(MOUSE_DOWN, event.motion.x, event.motion.y);

            } else if (event.type == SDL_MOUSEBUTTONUP) {
                isDown = false;
                eventFunc(MOUSE_UP, event.motion.x, event.motion.y);
            }
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
