#include "../include/dsm.h"
// #define VMRP

extern int32 mr_c_function_load(int32 code);

extern int (*MR_SPRINTF)(char *s, const char *format, ...);
#define mrc_sprintf MR_SPRINTF

void mrc_clearScreen(int32 r, int32 g, int32 b);
int32 mrc_drawText(char *pcText, int16 x, int16 y, uint8 r, uint8 g, uint8 b, int is_unicode, uint16 font);
void mrc_refreshScreen(int16 x, int16 y, uint16 w, uint16 h);

char buf[256];

#ifdef VMRP
extern void *mrc_malloc(int size);

static DSM_EXPORT_FUNCS *mythroad;
static DSM_REQUIRE_FUNCS *funcs;

void panic(char *msg) {
    mrc_clearScreen(0, 0, 0);
    mrc_drawText(msg, 0, 40, 255, 255, 255, 0, 1);
    mrc_refreshScreen(0, 0, 240, 320);
    while (1) {
    }
}

void br_log(char *msg) {
    // mrc_clearScreen(0, 0, 0);
    // mrc_drawText(msg, 0, 40, 255, 255, 255, 0, 1);
    // mrc_refreshScreen(0, 0, 240, 320);
}

void br_exit(void) {
    mrc_clearScreen(0, 0, 0);
    mrc_drawText("mythroad_exit.", 0, 40, 255, 255, 255, 0, 1);
    mrc_refreshScreen(0, 0, 240, 320);
}

void br_srand(uint32 seed) {
    // mrc_sand(seed);
}

int32 br_rand(void) {
    // return mrc_rand();
    return 0;
}

int32 br_mem_get(char **mem_base, uint32 *mem_len) {
    *mem_len = 300 * 1024;
    *mem_base = mrc_malloc(*mem_len);
    return 0;
}

int32 br_mem_free(char *mem, uint32 mem_len) {
    return 0;
}

void br_drawBitmap(uint16 *data, int16 x, int16 y, uint16 w, uint16 h) {
    extern void mrc_drawPoint(int16 x, int16 y, uint16 nativecolor);
    int32 i, j, xx, yy;
    uint16 color;
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            xx = x + i;
            yy = y + j;
            if (xx < 0 || yy < 0 || xx >= SCREEN_WIDTH || yy >= SCREEN_HEIGHT) {
                continue;
            }
            color = *(data + (xx + yy * SCREEN_WIDTH));
            mrc_drawPoint(xx, yy, color);
            mrc_refreshScreen(xx, yy, 1, 1);
        }
    }
}

int64 get_time_ms(void) {
    return 0;
}

int32 br_open(const char *filename, uint32 mode) {
    extern int32 mrc_open(const char *filename, uint32 mode);
    return mrc_open(filename, mode);
}

int32 br_mkDir(const char *path) {
    extern int32 mrc_mkDir(const char *name);
    return mrc_mkDir(path);
}

int32 br_close(int32 f) {
    extern int32 mrc_close(int32 f);
    return mrc_close(f);
}

int32 br_read(int32 f, void *p, uint32 l) {
    extern int32 mrc_read(int32 f, void *p, uint32 l);
    return mrc_read(f, p, l);
}

int32 br_write(int32 f, void *p, uint32 l) {
    extern int32 mrc_write(int32 f, void *p, uint32 l);
    return mrc_write(f, p, l);
}

int32 br_seek(int32 f, int32 pos, int method) {
    extern int32 mrc_seek(int32 f, int32 pos, int method);
    return mrc_seek(f, pos, method);
}

#endif

int32 mrc_init(void) {
    uint32 baseAddr = (uint32)mr_c_function_load;
    uint32 addr = (uint32)dsm_init;
    int16 y = 0;

    mrc_clearScreen(0, 0, 0);

    mrc_sprintf(buf, "VMRP_VER:%d", VMRP_VER);
    mrc_drawText(buf, 0, y, 255, 255, 255, 0, 1);

    y+=20;
    mrc_sprintf(buf, "entry addr:0x%X", baseAddr);
    mrc_drawText(buf, 0, y, 255, 255, 255, 0, 1);

    y+=20;
    mrc_sprintf(buf, "dsm_init addr:0x%X", addr);
    mrc_drawText(buf, 0, y, 255, 255, 255, 0, 1);

    y+=20;
    mrc_sprintf(buf, "dsm_init pos:0x%X", addr - baseAddr);
    mrc_drawText(buf, 0, y, 255, 255, 255, 0, 1);

    y+=20;
    mrc_sprintf(buf, "DSM_REQUIRE_FUNCS:0x%X", sizeof(DSM_REQUIRE_FUNCS));
    mrc_drawText(buf, 0, y, 255, 255, 255, 0, 1);

    y+=20;
    mrc_sprintf(buf, "DSM_EXPORT_FUNCS:0x%X", sizeof(DSM_EXPORT_FUNCS));
    mrc_drawText(buf, 0, y, 255, 255, 255, 0, 1);

    mrc_refreshScreen(0, 0, 240, 320);

#ifdef VMRP
    funcs = mrc_malloc(sizeof(DSM_REQUIRE_FUNCS));
    funcs->panic = panic;
    funcs->log = br_log;
    funcs->exit = br_exit;
    funcs->srand = br_srand;
    funcs->rand = br_rand;
    funcs->mem_get = br_mem_get;
    funcs->mem_free = br_mem_free;
    // funcs->timerStart = br_timerStart;
    // funcs->timerStop = br_timerStop;
    funcs->get_time_ms = get_time_ms;
    // funcs->getDatetime = br_getDatetime;
    // funcs->sleep = br_sleep;
    funcs->open = br_open;
    funcs->close = br_close;
    funcs->read = br_read;
    funcs->write = br_write;
    funcs->seek = br_seek;
    // funcs->info = br_info;
    // funcs->remove = br_remove;
    // funcs->rename = br_rename;
    funcs->mkDir = br_mkDir;
    // funcs->rmDir = br_rmDir;
    // funcs->opendir = br_opendir;
    // funcs->readdir = br_readdir;
    // funcs->closedir = br_closedir;
    // funcs->getLen = br_getLen;
    funcs->drawBitmap = br_drawBitmap;

    mythroad = dsm_init(funcs);

    mythroad->mr_start_dsm("mr.mrp");
#endif
    return (uint32)dsm_init;
}

int32 mrc_exitApp(void) {
    return MR_SUCCESS;
}

int32 mrc_event(int32 code, int32 param0, int32 param1) {
#ifdef VMRP
    return mythroad->mr_event(code, param0, param1);
#endif
    return MR_SUCCESS;
}

int32 mrc_pause() {
    return MR_SUCCESS;
}

int32 mrc_resume() {
    return MR_SUCCESS;
}

int32 mrc_extRecvAppEventEx(int32 code, int32 param0, int32 param1) {
    return MR_SUCCESS;
}

int32 mrc_extRecvAppEvent(int32 app, int32 code, int32 param0, int32 param1) {
    return MR_SUCCESS;
}
