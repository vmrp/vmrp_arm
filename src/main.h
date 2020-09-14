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
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

int getFileType(const char *name);
int getFileSize(const char *path);
void logPrint(char *level, char *tag, ...);

int64 get_uptime_ms(void);
int64 get_time_ms(void);


typedef struct _EmuEnv {
    int showFile;               //文件 I/O
    int showNet;                //网络
    int b_tsfInited;            //tsf 加载结果
    int64 dsmStartTime;         //虚拟机初始化时间，用来计算系统运行时间
    char *vm_mem_base;          //虚拟机内存地址
    int32 vm_mem_len;           //虚拟机内存大小
    char *vm_mem_end;           //虚拟机内存地址
    uint16 *screenBuffer;       //缓冲屏幕地址
} T_EMUENV;

typedef struct {
    char *title;
    char **items;
    int itemCount;
} T_MR_MENU, *PT_MR_MENU;

typedef int32 (*MR_CALLBACK)(int32 result);

//--- DSM 配置参数 ----------------------------
#define DSM_MAX_FILE_LEN 256
#define DSM_MAX_NAME_LEN 128

//------- 全局变量 -----------------------------------------
extern int SCRW;
extern int SCRH;

extern T_EMUENV gEmuEnv;  //API LOG 控制

extern mr_socket_struct mr_soc;
extern int dsmNetType;

//------- 宏函数 ----------------------------------------------

#define FREE_SET_NULL(ptr) \
    if (ptr) {             \
        free(ptr);         \
        ptr = NULL;        \
    }

#define MKDIR(as) mkdir(as, S_IRWXU | S_IRWXG | S_IRWXO)

#define CHECK_AND_REMOVE(as)               \
    do {                                   \
        if (MR_IS_FILE == getFileType(as)) \
            remove(as);                    \
    } while (0)

#define MR_CHECK_AND_REMOVE(as)        \
    do {                               \
        if (MR_IS_FILE == mr_info(as)) \
            mr_remove(as);             \
    } while (0)


////////// 平台相关个方法 N->J //////////////////////////////////
void emu_bitmapToscreen(uint16 *data, int x, int y, int w, int h);
int32 emu_timerStart(uint16 t);
int32 emu_timerStop(void);
void emu_finish();


extern int32 mr_start_dsmC(char *start_file, const char *entry);

#define isAddrValid(p) \
    ((void *)p >= (void *)gEmuEnv.vm_mem_base && (void *)p < (void *)gEmuEnv.vm_mem_end)

#endif
