#ifndef _EMULATOR_H
#define _EMULATOR_H

#include "./mr/include/type.h"
#include "./mr/include/mr_helper.h"
#include "dsm.h"

#define LOG_TAG "vmrp"
#define LOGI(...)  logPrint("INFO", LOG_TAG, __VA_ARGS__)
#define LOGW(...)  logPrint("WARN", LOG_TAG, __VA_ARGS__)
#define LOGE(...)  logPrint("ERROR", LOG_TAG, __VA_ARGS__)
#define LOGD(...)  logPrint("DEBUG", LOG_TAG, __VA_ARGS__)

#define panic(msg)          \
    do {                    \
        printf("%s\n",msg);        \
        while (1); \
    } while (0)

int getFileType(const char *name);
int getFileSize(const char *path);
void logPrint(char *level, char *tag, ...);

int64 get_time_ms(void);


typedef struct {
    char *title;
    char **items;
    int itemCount;
} T_MR_MENU, *PT_MR_MENU;

typedef int32 (*MR_CALLBACK)(int32 result);

//--- DSM 配置参数 ----------------------------
#define DSM_MAX_FILE_LEN 256

//------- 全局变量 -----------------------------------------
extern int SCRW;
extern int SCRH;


extern mr_socket_struct mr_soc;

//------- 宏函数 ----------------------------------------------


#define MKDIR(as) mkdir(as, S_IRWXU | S_IRWXG | S_IRWXO)




////////// 平台相关个方法 N->J //////////////////////////////////
void emu_bitmapToscreen(uint16 *data, int x, int y, int w, int h);
int32 emu_timerStart(uint16 t);
int32 emu_timerStop(void);
void emu_finish();


extern int32 mr_start_dsmC(char *start_file, const char *entry);


#endif
