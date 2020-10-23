#include "./include/dsm.h"

#include "./include/encode.h"
#include "./include/fixR9.h"
#include "./include/printf.h"
#include "./include/string.h"

#define DSM_MAX_FILE_LEN 256

#define MT6235

/*请不要修改这些值*/
#if (defined(MT6223P) || defined(MT6223) || defined(MT6223P_S00))
#define DSM_PLAT_VERSION (2) /*手机平台区分(1~99)*/
#elif (defined(MT6226) || defined(MT6226M) || defined(MT6226D))
#define DSM_PLAT_VERSION (4) /*手机平台区分(1~99)*/
#elif (defined(MT6228))
#define DSM_PLAT_VERSION (5) /*手机平台区分(1~99)*/
#elif (defined(MT6225))
#define DSM_PLAT_VERSION (3) /*手机平台区分(1~99)*/
#elif (defined(MT6230))
#define DSM_PLAT_VERSION (6) /*手机平台区分(1~99)*/
#elif (defined(MT6227) || defined(MT6227D))
#define DSM_PLAT_VERSION (7)
#elif (defined(MT6219))
#define DSM_PLAT_VERSION (1)
#elif (defined(MT6235) || defined(MT6235B))
#define DSM_PLAT_VERSION (8)
#elif (defined(MT6229))
#define DSM_PLAT_VERSION (9)
#elif (defined(MT6253) || defined(MT6253T))
#define DSM_PLAT_VERSION (10)
#elif (defined(MT6238))
#define DSM_PLAT_VERSION (11)
#elif (defined(MT6239))
#define DSM_PLAT_VERSION (12)
#else
#error PLATFORM NOT IN LIST PLEASE CALL SKY TO ADD THE PLATFORM
#endif

#ifdef DSM_IDLE_APP
#define DSM_FAE_VERSION (180) /*由平台组统一分配版本号，有需求请联系平台组*/
#else
#define DSM_FAE_VERSION (182) /*由平台组统一分配版本号，有需求请联系平台组*/
#endif

static DSM_REQUIRE_FUNCS *dsmInFuncs;
static uint32 dsmStartTime;  //虚拟机初始化时间，用来计算系统运行时间

//////////////////////////////////////////////////////////////////

void mr_panic(char *msg) {
    dsmInFuncs->panic(msg);
}

void mr_printf(const char *format, ...) {
    char printfBuf[512] = {0};
    char utf8Buf[1024] = {0};
    va_list params;

    va_start(params, format);
    vsnprintf_(printfBuf, sizeof(printfBuf), format, params);
    va_end(params);
    GBToUTF8String((uint8 *)printfBuf, (uint8 *)utf8Buf, sizeof(utf8Buf));
    dsmInFuncs->log(utf8Buf);
}

#define LOGI(fmt, ...) mr_printf("[INFO]" fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) mr_printf("[WARN]" fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) mr_printf("[ERROR]" fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) mr_printf("[DEBUG]" fmt, ##__VA_ARGS__)

///////////////////////////////////////////////////////////////////
#define EN_CHAR_H 16
#define EN_CHAR_W 8
#define CN_CHAR_H 16
#define CN_CHAR_W 16

// todo "上有名不"这四个字必定显示为错别字
static char font_sky16_bitbuf[32];
static int font_sky16_f;

static int xl_font_sky16_init() {  //字体初始化，打开字体文件
    font_sky16_f = mr_open("system/gb16.uc2", 0);
    if (font_sky16_f <= 0) {
        LOGW("%s", "font load fail");
        return -1;
    }
    LOGI("font load suc fd=%d", font_sky16_f);
    return 0;
}

static int xl_font_sky16_close() {  //关闭字体
    return mr_close(font_sky16_f);
}

//获取字体第n个点信息
static int getfontpix(char *buf, int n) {
    buf += n / 8;  //计算在第几个字节，从0开始
    //计算在第几位n%8
    return (128 >> (n % 8)) & *buf;
}

//获得字符的位图
static char *xl_font_sky16_getChar(uint16 id) {
    mr_seek(font_sky16_f, id * 32, 0);
    mr_read(font_sky16_f, font_sky16_bitbuf, 32);
    return font_sky16_bitbuf;
}

//画一个字
static char *xl_font_sky16_drawChar(uint16 id, int x, int y, uint16 color) {
    int y2 = y + 16;
    int n = 0, count;
    int ix = x;
    int iy;
    extern void _DrawPoint(int16 x, int16 y, uint16 nativecolor);

    mr_seek(font_sky16_f, id * 32, 0);
    mr_read(font_sky16_f, font_sky16_bitbuf, 32);
    for (iy = y; iy < y2; iy++) {
        ix = x;
        for (count = 0; count < 16; count++) {
            if (getfontpix(font_sky16_bitbuf, n))
                _DrawPoint(ix, iy, color);
            n++, ix++;
        }
    }
    return font_sky16_bitbuf;
}

//获取一个文字的宽高
static void xl_font_sky16_charWidthHeight(uint16 id, int32 *width, int32 *height) {
    if (id < 128) {
        if (width) *width = EN_CHAR_W;
        if (height) *height = EN_CHAR_H;
        return;
    }
    if (width) *width = CN_CHAR_W;
    if (height) *height = CN_CHAR_H;
}

/*
//获取单行文字的宽高
void xl_font_sky16_textWidthHeight(char *text, int32 *width, int32 *height) {
    int textIdx = 0;
    int id;
    int fontw = 0, fonth = 0;
    while (text[textIdx] != 0) {
        id = (text[textIdx] << 8) + (text[textIdx + 1]);
        xl_font_sky16_charWidthHeight(id, &fontw, &fonth);
        (*width) += fontw;
        (*height) += fonth;
        textIdx += 2;
    }
}
*/

int32 mr_exit(void) {
    LOGD("%s", "mr_exit() called by mythroad!");
    xl_font_sky16_close();
    dsmInFuncs->exit();
    return MR_SUCCESS;
}

#define MAKE_PLAT_VERSION(plat, ver, card, impl, brun) \
    (100000000 + (plat)*1000000 + (ver)*10000 + (card)*1000 + (impl)*10 + (brun))

int32 mr_getUserInfo(mr_userinfo *info) {
    if (info == NULL) {
        return MR_FAILED;
    }

    LOGI("%s", "mr_getUserInfo");

    memset2(info, 0, sizeof(mr_userinfo));
    strcpy2((char *)info->IMEI, "864086040622841");
    strcpy2((char *)info->IMSI, "460019707327302");
    strncpy2(info->manufactory, "vmrp", 7);
    strncpy2(info->type, "vmrp", 7);

    info->ver = 101000000 + DSM_PLAT_VERSION * 10000 + DSM_FAE_VERSION;
    //	info->ver = 116000000 + DSM_PLAT_VERSION * 10000 + DSM_FAE_VERSION; //SPLE
    //	info->ver = MAKE_PLAT_VERSION(1, 3, 0, 18, 0);

    memset2(info->spare, 0, sizeof(info->spare));

#if 1
    LOGI("imei = %s", info->IMEI);
    LOGI("imsi = %s", info->IMSI);
    LOGI("factory = %s", info->manufactory);
    LOGI("type = %s", info->type);
    LOGI("ver = %d", info->ver);
#endif

    LOGI("%s", "mr_getUserInfo suc!");
    return MR_SUCCESS;
}

int32 mr_cacheSync(void *addr, int32 len) {
    LOGW("mr_cacheSync(0x%p, %d)", addr, len);
#if defined(__arm__)
    // cacheflush((long)addr, (long)(addr + len), 0);
#endif
    return MR_SUCCESS;
}

int32 mr_mem_get(char **mem_base, uint32 *mem_len) {
    return dsmInFuncs->mem_get(mem_base, mem_len);
}

int32 mr_mem_free(char *mem, uint32 mem_len) {
    return dsmInFuncs->mem_free(mem, mem_len);
}

int32 mr_timerStart(uint16 t) {
    return dsmInFuncs->timerStart(t);
}

int32 mr_timerStop(void) {
    return dsmInFuncs->timerStop();
}

uint32 mr_getTime(void) {
    uint32 s;
    s = dsmInFuncs->get_uptime_ms() - dsmStartTime;
    LOGI("mr_getTime():%d", s);
    return s;
}

int32 mr_getDatetime(mr_datetime *datetime) {
    return dsmInFuncs->getDatetime(datetime);
}

int32 mr_sleep(uint32 ms) {
    return dsmInFuncs->sleep(ms);
}

///////////////////////// 文件操作接口 //////////////////////////////////////
#define MYTHROAD_PATH "mythroad/"
#define DSM_HIDE_DRIVE "mythroad/disk/"
#define DSM_DRIVE_A "mythroad/disk/a/"
#define DSM_DRIVE_B "mythroad/disk/b/"
#define DSM_DRIVE_X "mythroad/disk/x/"

static char dsmWorkPath[DSM_MAX_FILE_LEN] = MYTHROAD_PATH; /*当前工作路径 gb 编码*/

/*
 * 整理路径，将分隔符统一为sep，并清除连续的多个
 * 参数：路径(必须可读写)
 */
static char *formatPathString(char *path, char sep) {
    char *p, *q;
    int flag = 0;

    for (p = q = path; *p; p++) {
        if ('\\' == *p || '/' == *p) {
            if (0 == flag) {
                *q = sep;
                q++;
            }
            flag = 1;
        } else {
            *q = *p;
            q++;
            flag = 0;
        }
    }
    *q = '\0';
    return path;
}

static void SetDsmWorkPath(const char *path) {
    int l;
    strncpy2(dsmWorkPath, path, sizeof(dsmWorkPath) - 1);
    formatPathString(dsmWorkPath, '/');

    l = strlen2(dsmWorkPath);
    if (dsmWorkPath[l - 1] != '/') {
        dsmWorkPath[l] = '/';
        dsmWorkPath[l + 1] = '\0';
    }
    LOGW("SetDsmWorkPath():'%s'", dsmWorkPath);
}

static char dsmSwitchPathBuf[DSM_MAX_FILE_LEN + 10];
static int32 dsmSwitchPath(uint8 *input, int32 input_len, uint8 **output, int32 *output_len) {
    LOGI("dsmSwitchPath '%s', %d, %p, %p", input, input_len, output, output_len);
    /*
        功能：将SkyEngine的根目录切换至新目录。目录字符串如：”C:/App/”，第一个字符表示切换至的存储设备：（盘符不区分大小写，GB编码）
        第二、第三字符为“:/”，第四字符起为该存储设备上的目录名。
    */
    switch (input[0]) {
        case 'Z':  // 返回刚启动时路径
        case 'z':
            strcpy2(dsmWorkPath, MYTHROAD_PATH);
            break;

        case 'Y':
        case 'y': {  // 获取当前的路径设置，返回型如："C:/App/"（即必须符合上述输入标准），gb编码；
            char *p;
            if ((p = strstr2(dsmWorkPath, DSM_HIDE_DRIVE)) != NULL) {  //在A盘下
                p += strlen2(DSM_HIDE_DRIVE);                          //a/...
                if (p) {
                    if (*(p + 2))
                        snprintf_(dsmSwitchPathBuf, sizeof(dsmSwitchPathBuf), "%c:/%s", *p, (p + 2));
                    else
                        snprintf_(dsmSwitchPathBuf, sizeof(dsmSwitchPathBuf), "%c:/", *p);
                } else {
                    dsmInFuncs->panic("dsmWorkPath y ERROR!");
                }
            } else {
                snprintf_(dsmSwitchPathBuf, sizeof(dsmSwitchPathBuf), "c:/%s", dsmWorkPath);
            }
            LOGW("dsmSwitchPathBuf: y '%s'", dsmSwitchPathBuf);
            *output = (uint8 *)dsmSwitchPathBuf;
            *output_len = strlen2(dsmSwitchPathBuf);
            break;
        }

        case 'x':
        case 'X':  // 进入vm的根目录（后继子串参数无意义）。这个根目录必须放在用户不可见的，不能卸载的盘上。在这个根目录下可以保存一些设置信息，及收费信息等；
            strcpy2(dsmWorkPath, DSM_DRIVE_X);
            break;

        case 'a':
        case 'A': {  // A：  普通用户不可见（不可操作）存储盘；
            if (input_len > 3) {
                sprintf_(dsmSwitchPathBuf, "%s%s", DSM_DRIVE_A, input + 3);
            } else {
                sprintf_(dsmSwitchPathBuf, "%s", DSM_DRIVE_A);
            }
            SetDsmWorkPath(dsmSwitchPathBuf);
            break;
        }
        case 'b':
        case 'B': {  // B：  普通用户可操作存储盘（即可usb连接在PC上操作）；
            if (input_len > 3) {
                sprintf_(dsmSwitchPathBuf, "%s%s", DSM_DRIVE_B, input + 3);
            } else {
                sprintf_(dsmSwitchPathBuf, "%s", DSM_DRIVE_B);
            }
            SetDsmWorkPath(dsmSwitchPathBuf);
            break;
        }
        case 'c':
        case 'C':  // 外插存储设备，如mmc，sd，t-flash等；
            if (input_len > 3) {
                SetDsmWorkPath((char *)(input + 3));
            } else {
                dsmInFuncs->panic("dsmWorkPath c ERROR!");
            }
            break;

        default:
            LOGE("%s", "dsmSwitchPath() default");
            return MR_IGNORE;
    }

    return MR_SUCCESS;
}

char *get_filename(char *outputbuf, const char *filename) {
    char dsmFullPath[DSM_MAX_FILE_LEN + 10];
    snprintf_(dsmFullPath, sizeof(dsmFullPath), "%s%s", dsmWorkPath, filename);
    formatPathString(dsmFullPath, '/');
    GBToUTF8String((uint8 *)dsmFullPath, (uint8 *)outputbuf, DSM_MAX_FILE_LEN);
    return outputbuf;
}

int32 mr_open(const char *filename, uint32 mode) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    int32 ret;
    ret = dsmInFuncs->open(get_filename(fullpathname, filename), mode);
    LOGI("mr_open(%s,%d) fd is: %d", fullpathname, mode, ret);
    return ret;
}

int32 mr_close(int32 f) {
    int32 ret;
    ret = dsmInFuncs->close(f);
    LOGI("mr_close(%d): ret:%d", f, ret);
    return ret;
}

int32 mr_read(int32 f, void *p, uint32 l) {
    if (f != font_sky16_f) {
        LOGI("mr_read %d,%p,%d", f, p, l);
    }
    return dsmInFuncs->read(f, p, l);
}

int32 mr_write(int32 f, void *p, uint32 l) {
    LOGI("mr_write %d,%p,%d", f, p, l);
    return dsmInFuncs->write(f, p, l);
}

int32 mr_seek(int32 f, int32 pos, int method) {
    return dsmInFuncs->seek(f, pos, method);
}

int32 mr_info(const char *filename) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    return dsmInFuncs->info(get_filename(fullpathname, filename));
}

int32 mr_remove(const char *filename) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    int32 ret;
    ret = dsmInFuncs->remove(get_filename(fullpathname, filename));
    LOGI("mr_remove(%s) ret:%d", fullpathname, ret);
    return ret;
}

int32 mr_rename(const char *oldname, const char *newname) {
    char fullpathname_1[DSM_MAX_FILE_LEN] = {0};
    char fullpathname_2[DSM_MAX_FILE_LEN] = {0};
    get_filename(fullpathname_1, oldname);
    get_filename(fullpathname_2, newname);
    LOGI("mr_rename(%s to %s)", fullpathname_1, fullpathname_2);
    return dsmInFuncs->rename(fullpathname_1, fullpathname_2);
}

int32 mr_mkDir(const char *name) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    get_filename(fullpathname, name);
    LOGI("mr_mkDir(%s)", fullpathname);
    return dsmInFuncs->mkDir(fullpathname);
}

int32 mr_rmDir(const char *name) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    get_filename(fullpathname, name);
    LOGI("mr_rmDir(%s)", fullpathname);
    return dsmInFuncs->rmDir(fullpathname);
}

int32 mr_findGetNext(int32 search_handle, char *buffer, uint32 len) {
    char *d_name;
    d_name = dsmInFuncs->readdir(search_handle);
    if (d_name != NULL) {
        memset2(buffer, 0, len);
        UTF8ToGBString((uint8 *)d_name, (uint8 *)buffer, (int)len);
        LOGI("mr_findGetNext %d %s", search_handle, buffer);
        return MR_SUCCESS;
    }
    LOGI("mr_findGetNext %d (NULL)", search_handle);
    return MR_FAILED;
}

int32 mr_findStop(MR_SEARCH_HANDLE search_handle) {
    return dsmInFuncs->closedir(search_handle);
}

int32 mr_findStart(const char *name, char *buffer, uint32 len) {
    int32 ret;
    char fullpathname[DSM_MAX_FILE_LEN] = {0};

    get_filename(fullpathname, name);
    LOGI("mr_findStart(%s)", fullpathname);

    ret = dsmInFuncs->opendir(fullpathname);
    if (ret != MR_FAILED) {
        mr_findGetNext(ret, buffer, len);
        return ret;
    }
    LOGE("mr_findStart %s: opendir FAIL!", fullpathname);
    return MR_FAILED;
}

int32 mr_ferrno(void) {
    return MR_FAILED;
}

int32 mr_getLen(const char *filename) {
    char fullpathname[DSM_MAX_FILE_LEN] = {0};
    return dsmInFuncs->getLen(get_filename(fullpathname, filename));
}

int32 mr_getScreenInfo(mr_screeninfo *s) {
    LOGI("%s", "mr_getScreenInfo()");
    if (s) {
        s->width = SCREEN_WIDTH;
        s->height = SCREEN_HEIGHT;
        s->bit = 16;
    }
    return MR_SUCCESS;
}

void mr_drawBitmap(uint16 *bmp, int16 x, int16 y, uint16 w, uint16 h) {
    dsmInFuncs->drawBitmap(bmp, x, y, w, h);
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
    return MR_NET_ID_MOBILE;
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

int32 mr_rand(void) {
    return dsmInFuncs->rand();
}
//----------------------------------------------------
/*平台扩展接口*/
int32 mr_plat(int32 code, int32 param) {
    int32 ret = MR_IGNORE;

    LOGI("mr_plat code=%d param=%d", code, param);

    switch (code) {
        case MR_CONNECT: {  //1001
            ret = mr_getSocketState(param);
            break;
        }
        case MR_GET_RAND: {  //1211
            dsmInFuncs->srand(mr_getTime());
            ret = (MR_PLAT_VALUE_BASE + dsmInFuncs->rand() % param);
            break;
        }
        case MR_CHECK_TOUCH:  //1205是否支持触屏
            ret = MR_TOUCH_SCREEN;
            break;
        case MR_GET_HANDSET_LG:  //1206获取语言
            ret = MR_CHINESE;
            break;
        case 1218:  // 查询存储卡的状态
            ret = MR_MSDC_OK;
            break;
        default:
            LOGW("mr_plat(code:%d, param:%d) not impl!", code, param);
            break;
    }
    return ret;
}

/*增强的平台扩展接口*/
int32 mr_platEx(int32 code, uint8 *input, int32 input_len, uint8 **output, int32 *output_len, MR_PLAT_EX_CB *cb) {
    int32 ret = MR_IGNORE;
    LOGI("mr_platEx code=%d in=%p inlen=%d out=%p outlen=%p cb=%p", code, input, input_len, output, output_len, cb);

    switch (code) {
        case 1012:  //申请内部cache
        case 1013:  //释放内部cache
            break;
        case 1014: {  //申请拓展内存
            // *output_len = SCRW * SCRH * 4;
            // *output = malloc(*output_len);
            // LOGI("malloc exRam addr=%p len=%d", output, output_len);
            // ret= MR_SUCCESS;
            break;
        }
        case 1015: {  //释放拓展内存
            // LOGI("free exRam");
            // free(input);
            // ret= MR_SUCCESS;
            break;
        }
        case MR_TUROFFBACKLIGHT:  //关闭背光常亮
        case MR_TURONBACKLIGHT:   //开启背光常亮
            ret = MR_SUCCESS;
            break;
        case MR_SWITCHPATH:  //切换跟目录 1204
            ret = dsmSwitchPath(input, input_len, output, output_len);
            break;
            // case MR_GET_FREE_SPACE:

        case MR_CHARACTER_HEIGHT: {  // 1201
            static int32 wordInfo = (EN_CHAR_H << 24) | (EN_CHAR_W << 16) | (CN_CHAR_H << 8) | (CN_CHAR_W);
            *output = (unsigned char *)&wordInfo;
            *output_len = 4;
            ret = MR_SUCCESS;
            break;
        }

        case 1116: {  //获取编译时间
            static char buf[32];
            int l = snprintf_(buf, sizeof(buf), "%s %s", __TIME__, __DATE__);
            *output = (uint8 *)buf;  //"2013/3/21 21:36";
            *output_len = l + 1;
            LOGI("build time %s", buf);
            ret = MR_SUCCESS;
            break;
        }

        case 1224:  //小区信息ID
        case 1307:  //获取SIM卡个数，非多卡多待直接返回 MR_INGORE
            break;

        case 1017: {  //获得信号强度。
            static T_RX rx = {3, 5, 5, 1};
            *output = (uint8 *)&rx;
            *output_len = sizeof(T_RX);
            ret = MR_SUCCESS;
            break;
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
    return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int32 mr_initNetwork(MR_INIT_NETWORK_CB cb, const char *mode) {
    LOGI("mr_initNetwork(mod:%s)", mode);
    return MR_FAILED;
}

int32 mr_closeNetwork() {
    LOGI("%s", "mr_closeNetwork");
    return MR_FAILED;
}

int32 mr_getHostByName(const char *ptr, MR_GET_HOST_CB cb) {
    return MR_FAILED;
}

int32 mr_socket(int32 type, int32 protocol) {
    LOGI("mr_socket(type:%d, protocol:%d)", type, protocol);
    return MR_FAILED;
}

int32 mr_connect(int32 s, int32 ip, uint16 port, int32 type) {
    LOGI("mr_connect(s:%d, ip:%d, port:%d, type:%d)", s, ip, port, type);
    return MR_FAILED;
}

int32 mr_getSocketState(int32 s) {
    LOGI("getSocketState(%d)", s);
    return MR_FAILED;
}

int32 mr_closeSocket(int32 s) {
    LOGI("mr_closeSocket(%d)", s);
    return MR_FAILED;
}

int32 mr_recv(int32 s, char *buf, int len) {
    LOGI("mr_recv(%d)", s);
    return MR_FAILED;
}

int32 mr_send(int32 s, const char *buf, int len) {
    LOGI("mr_send %d %s", s, buf);
    return MR_FAILED;
}

int32 mr_recvfrom(int32 s, char *buf, int len, int32 *ip, uint16 *port) {
    LOGI("mr_recvfrom(%d,%s,%d,%d,%d)", s, buf, len, *ip, *port);
    return MR_FAILED;
}

int32 mr_sendto(int32 s, const char *buf, int len, int32 ip, uint16 port) {
    LOGI("mr_sendto(%d,%s,%d,%d,%d)", s, buf, len, ip, port);
    return MR_FAILED;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static DSM_EXPORT_FUNCS dsm_export_funcs;

DSM_EXPORT_FUNCS *dsm_init(DSM_REQUIRE_FUNCS *inFuncs) {
    dsmInFuncs = inFuncs;
    dsmStartTime = dsmInFuncs->get_uptime_ms();
    dsmInFuncs->mkDir(MYTHROAD_PATH);
    dsmInFuncs->mkDir(DSM_HIDE_DRIVE);
    dsmInFuncs->mkDir(DSM_DRIVE_A);
    dsmInFuncs->mkDir(DSM_DRIVE_B);
    dsmInFuncs->mkDir(DSM_DRIVE_X);

#ifdef DSM_FULL
    mr_tm_init();
    mr_baselib_init();
    mr_tablib_init();
    mr_socket_target_init();
    mr_tcp_target_init();
    mr_iolib_target_init();
    mr_strlib_init();
    mr_pluto_init();
#endif
    mythroad_init();
    xl_font_sky16_init();

    dsm_export_funcs.version = VMRP_VER;
    dsm_export_funcs.mr_start_dsm = mr_start_dsm;
    dsm_export_funcs.mr_pauseApp = mr_pauseApp;
    dsm_export_funcs.mr_resumeApp = mr_resumeApp;
    dsm_export_funcs.mr_timer = mr_timer;
    dsm_export_funcs.mr_event = mr_event;
    return &dsm_export_funcs;
}