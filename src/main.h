#ifndef _EMULATOR_H
#define _EMULATOR_H

#include "dsm.h"
#include "mr_helper.h"
#include "mr_types.h"
#include "utils.h"

#define LOG_TAG "mrpoid_jni"

#define LOGI(...) \
    ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) \
    ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) \
    ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define LOGD(...) \
    ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

typedef enum {
    EMU_MSG_STATR = 0x10000,

    EMU_MSG_MAX
} E_EMU_MSGID;

typedef struct _EmuEnv {
    int showFile;               //文件 I/O
    int showMrPlat;             //mr_plat/mr_platEx
    int showNet;                //网络
    int b_tsfInited;            //tsf 加载结果
    int font_sky_status;        //sky字体加载结果(风的影子)
    int64 dsmStartTime;         //虚拟机初始化时间，用来计算系统运行时间
    char *vm_mem_base;          //虚拟机内存地址
    int32 vm_mem_len;           //虚拟机内存大小
    char *vm_mem_end;           //虚拟机内存地址
    char *exMem;                //拓展内存地址
    uint16 *cacheScreenBuffer;  //缓冲屏幕地址
    uint16 *screenBuffer;       //缓冲屏幕地址
} T_EMUENV;

typedef struct {
    char *title;
    char **items;
    int itemCount;
} T_MR_MENU, *PT_MR_MENU;

typedef int32 (*MR_CALLBACK)(int32 result);

//--- 万能函数 标示 ----------------------------
#define EMU_FUNC_menuCreate "menuCreate"
#define EMU_FUNC_menuSetItem "menuSetItem"
#define EMU_FUNC_menuShow "menuShow"
#define EMU_FUNC_menuSetFocus "menuSetFocus"
#define EMU_FUNC_menuRelease "menuRelease"
#define EMU_FUNC_menuRefresh "menuRefresh"

#define EMU_FUNC_dialogCreate "dialogCreate"
#define EMU_FUNC_dialogRelease "dialogRelease"
#define EMU_FUNC_dialogRefresh "dialogRefresh"

#define EMU_FUNC_textCreate "textCreate"
#define EMU_FUNC_textRelease "textRelease"
#define EMU_FUNC_textRefresh "textRefresh"

//#define EMU_FUNC_ ""

//--- 获取系统信息的 key ----------------------------
#define SYSINFO_NETTYPE "netType"
#define SYSINFO_IMSI "imsi"
#define SYSINFO_IMEI "imei"

//--- DSM 配置参数 ----------------------------
#define SCNBIT 16
#define DSM_MAX_FILE_LEN 256
#define DSM_MAX_NAME_LEN 128
#define MAX_IMEI_LEN 15
#define MAX_IMSI_LEN 15
#define MAX_SMS_CENTER_LEN 15

//------- 全局变量 -----------------------------------------
extern int SCNW;
extern int SCNH;
#define screenW SCNW
#define screenH SCNH

extern T_EMUENV gEmuEnv;  //API LOG 控制

extern int showApiLog;
extern uint16 *cacheScreenBuffer;  //缓冲屏幕地址
extern mr_socket_struct mr_soc;
extern T_DSM_MEDIA_PLAY dsmMediaPlay;  //音乐播放接口回调
extern int dsmNetType;
extern char dsmSmsCenter[MAX_SMS_CENTER_LEN + 1];

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

///////// 注册方法 J->N /////////////////////////////////////////
extern void mr_getMemoryInfo(uint32 *total, uint32 *free, uint32 *top);

////////// 平台相关个方法 N->J //////////////////////////////////
void emu_bitmapToscreen(uint16 *data, int x, int y, int w, int h);
int32 emu_timerStart(uint16 t);
int32 emu_timerStop(void);

int32 emu_showEdit(const char *title, const char *text, int type, int max_size);
const char *emu_getEditInputContent(int32 editHd);
void emu_releaseEdit(int32 editHd);

void emu_finish();
void emu_palySound(const char *path, int loop);
void emu_stopSound(int id);
void emu_musicLoadFile(const char *path);
int emu_musicCMD(int cmd, int arg0, int arg1);

void N2J_readTsfFont(uint8 **outbuf, int32 *outlen);

void emu_getImageSize(const char *path, int *w, int *h);
void emu_drawImage(const char *path, int x, int y, int w, int h);

void SetDsmPath(const char *path);

extern int32 mr_start_dsmC(char *start_file, const char *entry);

#define isAddrValid(p) \
    ((void *)p >= (void *)gEmuEnv.vm_mem_base && (void *)p < (void *)gEmuEnv.vm_mem_end)

#endif
