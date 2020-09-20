#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>

#include "./mr/include/encode.h"
#include "font_sky16_2.h"
#include "main.h"

static int32 dsmSwitchPath(uint8 *input, int32 input_len, uint8 **output, int32 *output_len);
static void DsmPathInit();

extern void DsmSocketInit();
extern void DsmSocketClose();
void dsmRestoreRootDir();

static uint16 *screenBuf;

#define DSM_MEM_SIZE (5 * 1024 * 1024)  //DSM内存大小

//-- log 缓冲区 -------------------------------
#define PRINTF_BUF_LEN 1024
static char printfBuf[PRINTF_BUF_LEN + 2] = {0};
static char utf8Buf[PRINTF_BUF_LEN * 2 + 2] = {0};

static T_DSM_DISK_INFO dsmDiskInfo;
static int dsmNetWorkID;

int dsmNetType;

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

void dsm_init(uint16 *scrBuf) {
    screenBuf = scrBuf;
    DsmPathInit();
    DsmSocketInit();
    handleInit();
    dsmNetWorkID = MR_NET_ID_MOBILE;
    dsmNetType = NETTYPE_CMWAP;
}

int32 mr_exit(void) {
    LOGD("mr_exit() called by mythroad!");

    DsmSocketClose();
    emu_finish();
    return MR_SUCCESS;
}

void dpoint(int x, int y, int color) {
    if (x < 0 || x >= SCRW || y < 0 || y >= SCRH)
        return;
    *(screenBuf + y * SCRW + x) = color;
}

#define MAKE_PLAT_VERSION(plat, ver, card, impl, brun) \
    (100000000 + (plat)*1000000 + (ver)*10000 + (card)*1000 + (impl)*10 + (brun))

int32 mr_getUserInfo(mr_userinfo *info) {
    if (info == NULL)
        return MR_FAILED;

    LOGI("mr_getUserInfo");

    memset(info, 0, sizeof(mr_userinfo));
    strcpy(info->IMEI, "864086040622841");
    strcpy(info->IMSI, "460019707327302");
    strncpy(info->manufactory, "vmrp", 7);
    strncpy(info->type, "vmrp", 7);

    info->ver = 101000000 + DSM_PLAT_VERSION * 10000 + DSM_FAE_VERSION;
    //	info->ver = 116000000 + DSM_PLAT_VERSION * 10000 + DSM_FAE_VERSION; //SPLE
    //	info->ver = MAKE_PLAT_VERSION(1, 3, 0, 18, 0);

    memset(info->spare, 0, sizeof(info->spare));

#if 1
    LOGI("imei = %s", info->IMEI);
    LOGI("imsi = %s", info->IMSI);
    LOGI("factory = %s", info->manufactory);
    LOGI("type = %s", info->type);
    LOGI("ver = %d", info->ver);
#endif

    LOGI("mr_getUserInfo suc!");

    return MR_SUCCESS;
}

int32 mr_cacheSync(void *addr, int32 len) {
    LOGI("mr_cacheSync(%#p, %d)", addr, len);
#if defined(__arm__)
    // cacheflush((long)addr, (long)(addr + len), 0);
#endif
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

    gEmuEnv.vm_mem_base = buffer;
    gEmuEnv.vm_mem_len = len;
    gEmuEnv.vm_mem_end = buffer + len;

    LOGE("mr_mem_get base=%p len=%x end=%p =================", gEmuEnv.vm_mem_base, len, gEmuEnv.vm_mem_end);

    return MR_SUCCESS;
}

int32 mr_mem_free(char *mem, uint32 mem_len) {
    free(mem);
    gEmuEnv.vm_mem_base = NULL;
    gEmuEnv.vm_mem_len = 0;
    LOGI("mr_mem_free");
    return MR_SUCCESS;
}

int32 pageMalloc(void **out, int32 *outLen, uint32 needLen) {
    char *buf;
    int pagesize, pagecount;

    pagesize = sysconf(_SC_PAGE_SIZE);
    if (pagesize == -1)
        panic("sysconf");

    pagecount = needLen / pagesize + 1;
    needLen = pagesize * pagecount;
    buf = memalign(pagesize, needLen);
    if (buf == NULL)
        panic("memalign");

    if (mprotect(buf, needLen, PROT_EXEC | PROT_WRITE | PROT_READ) == -1) {
        free(buf);
        panic("mprotect");
    }
    *out = buf;
    *outLen = needLen;
    return MR_SUCCESS;
}

void dsmGB2UCS2(char *src, char *dest) {
    gbToUCS2((uint8 *)src, (uint8 *)dest);
}

int mr_sprintf(char *buf, const char *fmt, ...) {
    va_list vars;
    int ret;

    va_start(vars, fmt);
    ret = vsprintf(buf, fmt, vars);
    va_end(vars);
    return ret;
}

void mr_printf(const char *format, ...) {
    if (!format) {
        panic("mr_printf null");
        return;
    }

    va_list params;
    int len;

    va_start(params, format);
    len = vsnprintf(printfBuf, PRINTF_BUF_LEN, format, params);
    va_end(params);

    GBToUTF8String((uint8 *)printfBuf, (uint8 *)utf8Buf, sizeof(utf8Buf));
    LOGI("mr_printf(): %s", utf8Buf);
}

int32 mr_timerStart(uint16 t) {
    emu_timerStart(t);
    return MR_SUCCESS;
}

int32 mr_timerStop(void) {
    emu_timerStop();
    return MR_SUCCESS;
}

uint32 mr_getTime(void) {
    uint32 s = get_time_ms() - gEmuEnv.dsmStartTime;
    LOGI("mr_getTime():%d", s);
    return s;
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

int32 mr_sleep(uint32 ms) {
    LOGI("mr_sleep(%d)", ms);
    usleep(ms * 1000);  //注意 usleep 传的是 微秒 ，所以要 *1000
    return MR_SUCCESS;
}

///////////////////////// 文件操作接口 //////////////////////////////////////
static char SDPath[DSM_MAX_FILE_LEN / 2 + 1] = SDCARD_PATH;        /*SD卡根路径 gb 编码*/
static char dsmPath[DSM_MAX_FILE_LEN / 2 + 1] = DSM_ROOT_PATH;     /*mythroad工作目录（相对于SD卡路径） gb 编码*/
static char dsmWorkPath[DSM_MAX_FILE_LEN / 2 + 1] = DSM_ROOT_PATH; /*当前工作路径 gb 编码*/

static char filenamebuf[DSM_MAX_FILE_LEN + 1] = {0};

#define CHAR_DIR_SEP '/'

void DsmPathInit(void) {
    char buf[DSM_MAX_FILE_LEN * 2 + 1] = {0};
    char buf2[DSM_MAX_FILE_LEN * 2 + 1] = {0};

    snprintf(buf, sizeof(buf), "%s", SDPath);  //sd卡根目录
    GBToUTF8String((uint8 *)buf, (uint8 *)buf2, sizeof(buf2));
    LOGI("checkdir %s", buf2);
    if (MR_IS_DIR != getFileType(buf2))
        MKDIR(buf2);

    snprintf(buf, sizeof(buf), "%s%s", SDPath, dsmPath);  //mythroad 目录
    GBToUTF8String((uint8 *)buf, (uint8 *)buf2, sizeof(buf2));
    LOGI("checkdir %s", buf2);
    if (MR_IS_DIR != getFileType(buf2))
        MKDIR(buf2);

    snprintf(buf, sizeof(buf), "%s%s%s", SDPath, dsmPath, DSM_HIDE_DRIVE);
    GBToUTF8String((uint8 *)buf, (uint8 *)buf2, sizeof(buf2));
    LOGI("checkdir %s", buf2);
    if (MR_IS_DIR != getFileType(buf2))
        MKDIR(buf2);

    snprintf(buf, sizeof(buf), "%s%s%s%s", SDPath, dsmPath, DSM_HIDE_DRIVE, DSM_DRIVE_A);
    GBToUTF8String((uint8 *)buf, (uint8 *)buf2, sizeof(buf2));
    LOGI("checkdir %s", buf2);
    if (MR_IS_DIR != getFileType(buf2))
        MKDIR(buf2);

    snprintf(buf, sizeof(buf), "%s%s%s%s", SDPath, dsmPath, DSM_HIDE_DRIVE, DSM_DRIVE_B);
    GBToUTF8String((uint8 *)buf, (uint8 *)buf2, sizeof(buf2));
    LOGI("checkdir %s", buf2);
    if (MR_IS_DIR != getFileType(buf2))
        MKDIR(buf2);

    snprintf(buf, sizeof(buf), "%s%s%s%s%s", SDPath, dsmPath, DSM_HIDE_DRIVE, DSM_DRIVE_A, DSM_ROOT_PATH_SYS);
    GBToUTF8String((uint8 *)buf, (uint8 *)buf2, sizeof(buf2));
    LOGI("checkdir %s", buf2);
    if (MR_IS_DIR != getFileType(buf2))
        MKDIR(buf2);
}

/*
 * 整理路径，将分隔符统一为sep，并清除连续的多个
 * 参数：路径(必须可读写)
 */
char *FormatPathString(char *path, char sep) {
    char *p, *q;
    int flag = 0;

    if (NULL == path)
        return NULL;

    for (p = q = path; '\0' != *p; p++) {
        if ('\\' == *p || '/' == *p) {
            if (0 == flag)
                *q++ = sep;
            flag = 1;
        } else {
            *q++ = *p;
            flag = 0;
        }
    }

    *q = '\0';

    return path;
}

void SetDsmSDPath(const char *path) {
    LOGI("old sdpath %s", SDPath);

    strncpy(SDPath, path, sizeof(SDPath) - 1);
    FormatPathString(SDPath, CHAR_DIR_SEP);

    int l = strlen(SDPath);
    if (SDPath[l - 1] != '/') {
        SDPath[l] = '/';
        SDPath[l + 1] = '\0';
    }
    LOGI("new sdpath %s", SDPath);

    char gbBuf[DSM_MAX_FILE_LEN + 1];
    UTF8ToGBString(SDPath, gbBuf, DSM_MAX_FILE_LEN);

    strncpy(SDPath, gbBuf, sizeof(SDPath) - 1);
}

static void SetDsmWorkPath_inner(const char *path) {
    strncpy(dsmWorkPath, path, sizeof(dsmWorkPath) - 1);
    FormatPathString(dsmWorkPath, '/');

    int l = strlen(dsmWorkPath);
    if (dsmWorkPath[l - 1] != '/') {
        dsmWorkPath[l] = '/';
        dsmWorkPath[l + 1] = '\0';
    }
    //检查并创建目录
}

static int32 dsmSwitchPath(uint8 *input, int32 input_len, uint8 **output, int32 *output_len) {
    LOGI("dsmSwitchPath %s,%d, %p,%p", input, input_len, output, output_len);

    if (input == NULL)
        return MR_FAILED;

    input_len = strlen((char *)input);
    if (input_len > (DSM_MAX_FILE_LEN - 3))
        return MR_FAILED;

    switch (input[0]) {
        case 'Z':  //返回刚启动时路径
        case 'z':
            strcpy(dsmWorkPath, dsmPath);
            break;

        case 'Y':  //获取当前工作绝对路径
        case 'y': {
            static char buf[DSM_MAX_FILE_LEN];

            memset(buf, 0, sizeof(buf));

            char *p;

            /**
			 * 此处用 c:/ a:/ b:/ 代替真正SD卡路径，避免mrp层空间不足死机
			 */
            if ((p = strstr(dsmWorkPath, DSM_HIDE_DRIVE)) != NULL) {  //在A盘下
                p += strlen(DSM_HIDE_DRIVE);                          //a/...
                if (p) {
                    if (*(p + 2))
                        snprintf(buf, sizeof(buf), "%c:/%s", *p, (p + 2));
                    else
                        snprintf(buf, sizeof(buf), "%c:/", *p);
                } else {
                    //说明不是 .disk/a/ 形式，未知错误
                    LOGE("dsmWorkPath ERROR!");

                    strcpy(buf, "a:/");  //给他个默认值
                }
            } else {
                snprintf(buf, sizeof(buf), "c:/%s", dsmWorkPath);
            }

            //			LOGI("dsmWorkPath:%s", buf);

            *output = (uint8 *)buf;
            *output_len = strlen(buf);

            break;
        }

        case 'X':  //进入DSM隐藏根目录
        case 'x': {
            snprintf(filenamebuf, sizeof(filenamebuf), "%s%s%s%s",
                     dsmPath, DSM_HIDE_DRIVE, DSM_DRIVE_A, DSM_ROOT_PATH_SYS);
            SetDsmWorkPath_inner(filenamebuf);
            break;
        }

        default: {
            char buf[DSM_MAX_FILE_LEN + 1] = {0};

            if (input_len > DSM_MAX_FILE_LEN)
                return MR_FAILED;

            if (*input == 'A' || *input == 'a') {  //A 盘
                if (input_len > 3) {
                    sprintf(buf, "%s%s%s%s", dsmPath, DSM_HIDE_DRIVE, DSM_DRIVE_A, input + 3);
                } else {
                    sprintf(buf, "%s%s%s", dsmPath, DSM_HIDE_DRIVE, DSM_DRIVE_A);
                }
                SetDsmWorkPath_inner(buf);
            } else if (*input == 'B' || *input == 'b') {  //B 盘
                if (input_len > 3) {
                    sprintf(buf, "%s%s%s%s", dsmPath, DSM_HIDE_DRIVE, DSM_DRIVE_B, input + 3);
                } else {
                    sprintf(buf, "%s%s%s", dsmPath, DSM_HIDE_DRIVE, DSM_DRIVE_B);
                }
                SetDsmWorkPath_inner(buf);
            } else {  //C 盘
                if (input_len > 3) {
                    sprintf(buf, "%s", input + 3);
                    SetDsmWorkPath_inner(buf);
                } else {
                    SetDsmWorkPath_inner("");
                }
            }
            break;
        }
    }

    return MR_SUCCESS;
}

char *get_filename(char *outputbuf, const char *filename) {
    char dsmFullPath[DSM_MAX_FILE_LEN + 1];
    snprintf(dsmFullPath, sizeof(dsmFullPath), "%s%s%s", SDPath, dsmWorkPath, filename);
    FormatPathString(dsmFullPath, '/');
    GBToUTF8String(dsmFullPath, outputbuf, DSM_MAX_FILE_LEN);
    return outputbuf;
}

int startWith(const char *str, const char *s) {
    int l = strlen(s);
    //若参数s1 和s2 字符串相同则返回0。s1 若大于s2 则返回大于0 的值，s1 若小于s2 则返回小于0 的值。
    return (l > 0 && 0 == strncasecmp(str, s, l));
}

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
        LOGE("mr_close(%d) err, %d", f, errno);
        return MR_FAILED;
    }
    LOGI("mr_close(%d) suc", f);
    return MR_SUCCESS;
}

int32 mr_read(MR_FILE_HANDLE f, void *p, uint32 l) {
    extern int font_sky16_f;
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
        LOGE("mr_read(%d) err, %d", f, errno);
        return MR_FAILED;
    }
    return readnum;
}

int32 mr_write(MR_FILE_HANDLE f, void *p, uint32 l) {
    LOGI("mr_write %d,%p,%d", f, p, l);
    int32 writenum = write(int32ToHandle(f), p, (size_t)l);
    if (writenum < 0) {
        LOGE("mr_write(%d) err, %d", f, errno);
        return MR_FAILED;
    }
    return writenum;
}

int32 mr_seek(MR_FILE_HANDLE f, int32 pos, int method) {
    off_t ret = lseek(int32ToHandle(f), (off_t)pos, method);
    if (ret < 0) {
        LOGE("mr_seek(%d,%d) err, %d", f, pos, errno);
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
    if (ret != 0 && errno != 2) {
        LOGE("mr_remove(%s) err, ret=%d, errno=%d", fullpathname, ret, errno);
        return MR_FAILED;
    }
    LOGI("mr_remove(%s) suc", fullpathname);
    return MR_SUCCESS;
}

int32 mr_rename(const char *oldname, const char *newname) {
    char fullpathname_1[DSM_MAX_FILE_LEN] = {0};
    char fullpathname_2[DSM_MAX_FILE_LEN] = {0};
    int ret;

    LOGI("mr_rename(%s to %s)", oldname, newname);

    get_filename(fullpathname_1, oldname);
    get_filename(fullpathname_2, newname);
    ret = rename(fullpathname_1, fullpathname_2);
    if (ret != 0) {
        LOGE("mr_rename(%s to %s) err! errno=%d", fullpathname_1, fullpathname_2, errno);
        return MR_FAILED;
    }
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
    char fullpathname2[DSM_MAX_FILE_LEN] = {0};
    int ret;

    get_filename(fullpathname, name);

    //删除权限
    if (strcasecmp(fullpathname, SDCARD_PATH) == 0) {
        LOGE("Has no right to delete this directory:%s ", fullpathname);
        return MR_FAILED;
    }

    sprintf(fullpathname2, "%s%s", SDCARD_PATH, DSM_ROOT_PATH);
    if (strcasecmp(fullpathname, fullpathname2) == 0) {
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
            memset(buffer, 0, len);
            UTF8ToGBString(pDt->d_name, buffer, len);
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

    memset(buffer, 0, len);
    if ((pDt = readdir(pDir)) != NULL) {
        LOGI("mr_findGetNext %d %s", search_handle, pDt->d_name);
        UTF8ToGBString(pDt->d_name, buffer, len);
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

int32 mr_ferrno(void) {
    return MR_FAILED;
}

int32 mr_getLen(const char *filename) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    struct stat s1;
    int ret = stat(get_filename(fullpathname, filename), &s1);
    if (ret != 0)
        return -1;
    return s1.st_size;
}

int32 mr_getScreenInfo(mr_screeninfo *s) {
    LOGI("mr_getScreenInfo()");
    if (s) {
        s->width = SCRW;
        s->height = SCRH;
        s->bit = 16;
    }
    return MR_SUCCESS;
}

void mr_drawBitmap(uint16 *bmp, int16 x, int16 y, uint16 w, uint16 h) {
    emu_bitmapToscreen(bmp, x, y, w, h);
}

const char *mr_getCharBitmap(uint16 ch, uint16 fontSize, int *width, int *height) {
    xl_font_sky16_charWidthHeight(ch, width, height);
    return xl_font_sky16_getChar(ch);
}

void mr_platDrawChar(uint16 ch, int32 x, int32 y, uint32 color) {
    xl_font_sky16_drawChar(ch, x, y, (uint16)color);
}

int32 mr_startShake(int32 ms) {
    return MR_SUCCESS;
}

int32 mr_stopShake() {
    return MR_SUCCESS;
}

int32 mr_playSound(int type, const void *data, uint32 dataLen, int32 loop) {
    return MR_FAILED;
}

int32 mr_stopSound(int type) {
    return MR_SUCCESS;
}

int32 mr_sendSms(char *pNumber, char *pContent, int32 encode) {
    LOGI("mr_sendSms(%s)", pNumber);
    return MR_SUCCESS;
}
void mr_call(char *number) {
    LOGI("mr_call(%s)", number);
}

int32 mr_getNetworkID(void) {
    LOGI("mr_getNetworkID %d", dsmNetWorkID);
    return dsmNetWorkID;
}

void mr_connectWAP(char *wap) {
    LOGI("mr_connectWAP(%s)", wap);
}

int32 mr_menuCreate(const char *title, int16 num) {
    return MR_FAILED;
}

int32 mr_menuSetItem(int32 hd, const char *text, int32 index) {
    return MR_FAILED;
}

int32 mr_menuShow(int32 menu) {
    return MR_IGNORE;
}

int32 mr_menuSetFocus(int32 menu, int32 index) {
    return MR_IGNORE;
}

int32 mr_menuRelease(int32 menu) {
    return MR_IGNORE;
}

int32 mr_menuRefresh(int32 menu) {
    return MR_IGNORE;
}

int32 mr_dialogCreate(const char *title, const char *text, int32 type) {
    return MR_IGNORE;
}

int32 mr_dialogRelease(int32 dialog) {
    return MR_IGNORE;
}

int32 mr_dialogRefresh(int32 dialog, const char *title, const char *text, int32 type) {
    return MR_FAILED;
}

int32 mr_textCreate(const char *title, const char *text, int32 type) {
    return MR_IGNORE;
}

int32 mr_textRelease(int32 text) {
    return MR_IGNORE;
}

int32 mr_textRefresh(int32 handle, const char *title, const char *text) {
    return MR_IGNORE;
}

int32 mr_editCreate(const char *title, const char *text, int32 type, int32 max_size) {
    return MR_FAILED;
}

int32 mr_editRelease(int32 edit) {
    return MR_SUCCESS;
}

const char *mr_editGetText(int32 edit) {
    return NULL;
}

int32 mr_winCreate(void) {
    return MR_IGNORE;
}
int32 mr_winRelease(int32 win) {
    return MR_IGNORE;
}

//----------------------------------------------------
/*平台扩展接口*/
int32 mr_plat(int32 code, int32 param) {
    LOGI("mr_plat code=%d param=%d", code, param);

    switch (code) {
        case MR_CONNECT: {  //1001
            return mr_getSocketState(param);
        }
        case MR_GET_RAND: {  //1211
            srand(mr_getTime());
            return (MR_PLAT_VALUE_BASE + rand() % param);
        }
        case MR_CHECK_TOUCH:  //1205是否支持触屏
            return MR_TOUCH_SCREEN;
        case MR_GET_HANDSET_LG:  //1206获取语言
            return MR_CHINESE;
        case 1218:  // 查询存储卡的状态
            return MR_MSDC_OK;
        default:
            LOGW("mr_plat(code:%d, param:%d) not impl!", code, param);
            break;
    }

    return MR_IGNORE;
}

/*增强的平台扩展接口*/
int32 mr_platEx(int32 code, uint8 *input, int32 input_len, uint8 **output, int32 *output_len, MR_PLAT_EX_CB *cb) {
    LOGI("mr_platEx code=%d in=%p inlen=%d out=%p outlen=%p cb=%p", code, input, input_len, output, output_len, cb);

    switch (code) {
        case MR_MALLOC_EX: {  //1001申请屏幕缓冲区，这里给的值即 VM 的屏幕缓冲区
            *output = (uint8 *)screenBuf;
            *output_len = SCRW * SCRH * 2;
            LOGD("MR_MALLOC_EX ram2 addr=%p l=%d", output, *output_len);
            return MR_SUCCESS;
        }
        case MR_MFREE_EX: {  //1002
            return MR_SUCCESS;
        }
        case 1012:  //申请内部cache
        case 1013:  //释放内部cache
            return MR_IGNORE;
        case 1014: {  //申请拓展内存
            // *output_len = SCRW * SCRH * 4;
            // *output = malloc(*output_len);
            // LOGI("malloc exRam addr=%p len=%d", output, output_len);
            // return MR_SUCCESS;
            return MR_IGNORE;
        }
        case 1015: {  //释放拓展内存
            // LOGI("free exRam");
            // free(input);
            // return MR_SUCCESS;
            return MR_IGNORE;
        }
        case MR_TUROFFBACKLIGHT:  //关闭背光常亮
        case MR_TURONBACKLIGHT:   //开启背光常亮
            return MR_SUCCESS;

        case MR_SWITCHPATH:  //切换跟目录 1204
            return dsmSwitchPath(input, input_len, output, output_len);

        case MR_UCS2GB: {  //1207
            if (!input || input_len == 0) {
                LOGE("mr_platEx(1207) input err");
                return MR_FAILED;
            }

            if (!*output) {
                LOGE("mr_platEx(1207) ouput err");
                return MR_FAILED;
            }

            int len = UCS2_strlen(input);
            char *buf = malloc(len + 2);

            int gbBufLen = len + 1;
            char *gbBuf = malloc(gbBufLen);

            memcpy(buf, input, len + 2);
            UCS2ByteRev(buf);
            UCS2ToGBString((uint16 *)buf, gbBuf, gbBufLen);

            strcpy(*output, gbBuf);
            /**
				 * output_len 返回的GB字符串缓冲的长度。
				 *
				 * output   	转换成功以后的bg2312编码字符串存放缓冲区指针，缓冲区的内存由应用调用者提供并管理、释放。
				 * output_len   output缓冲区的长度，单位字节数
				 *
				 * 2013-3-25 16:29:21 2013-3-25 16:51:44
				 */
            //				if(output_len)
            //					*output_len = strlen(gbBuf);

            free(buf);
            return MR_SUCCESS;
        }

        case MR_CHARACTER_HEIGHT: {  // 1201
            static int32 wordInfo = (EN_CHAR_H << 24) | (EN_CHAR_W << 16) | (CN_CHAR_H << 8) | (CN_CHAR_W);
            *output = (unsigned char *)&wordInfo;
            *output_len = 4;
            return MR_SUCCESS;
        }

        case 1116: {  //获取编译时间
            static char buf[32];
            int l = snprintf(buf, sizeof(buf), "%s %s", __TIME__, __DATE__);
            *output = (uint8 *)buf;  //"2013/3/21 21:36";
            *output_len = l + 1;
            LOGI("build time %s", buf);
            return MR_SUCCESS;
        }

        case 1224:  //小区信息ID
        case 1307:  //获取SIM卡个数，非多卡多待直接返回 MR_INGORE
            return MR_IGNORE;

        case 1017: {  //获得信号强度。
            static T_RX rx = {3, 5, 5, 1};
            *output = (uint8 *)&rx;
            *output_len = sizeof(T_RX);
            return MR_SUCCESS;
        }

        default: {
            LOGW("mr_platEx(code=%d, in=%#p, inlen=%d) not impl!", code, input, input_len);
            break;
        }
    }

    // int cmd = code / 10;
    // switch (cmd) {
    //     case MR_MEDIA_INIT:              //201
    //     case MR_MEDIA_FILE_LOAD:         //202
    //     case MR_MEDIA_BUF_LOAD:          //203
    //     case MR_MEDIA_PLAY_CUR_REQ:      //204
    //     case MR_MEDIA_PAUSE_REQ:         //205
    //     case MR_MEDIA_RESUME_REQ:        //206
    //     case MR_MEDIA_STOP_REQ:          //207
    //     case MR_MEDIA_CLOSE:             //208
    //     case MR_MEDIA_GET_STATUS:        //209
    //     case MR_MEDIA_SETPOS:            //210
    //     case MR_MEDIA_GET_TOTAL_TIME:    //212
    //     case MR_MEDIA_GET_CURTIME:       //213
    //     case MR_MEDIA_GET_CURTIME_MSEC:  //215
    //     case MR_MEDIA_FREE:              //216
    //     default:
    //         LOGW("mr_platEx(code=%d, input=%p, il=%d) not impl!", code, (void *)input, input_len);
    //         break;
    // }

    return MR_IGNORE;
}
