#include "../include/dsm.h"

extern int32 mr_c_function_load(int32 code);

extern int (*MR_SPRINTF)(char *s, const char *format, ...);
#define mrc_sprintf MR_SPRINTF

void mrc_clearScreen(int32 r, int32 g, int32 b);
int32 mrc_drawText(char *pcText, int16 x, int16 y, uint8 r, uint8 g, uint8 b, int is_unicode, uint16 font);
void mrc_refreshScreen(int16 x, int16 y, uint16 w, uint16 h);

int32 mrc_init(void) {
    char buf[256];
    uint32 baseAddr = (uint32)mr_c_function_load;
    uint32 addr = (uint32)dsm_init;

    mrc_clearScreen(0, 0, 0);

    mrc_sprintf(buf, "entry addr:0x%X", baseAddr);
    mrc_drawText(buf, 0, 0, 255, 255, 255, 0, 1);

    mrc_sprintf(buf, "dsm_init addr:0x%X", addr);
    mrc_drawText(buf, 0, 20, 255, 255, 255, 0, 1);

    mrc_sprintf(buf, "dsm_init pos:0x%X", addr - baseAddr);
    mrc_drawText(buf, 0, 40, 255, 255, 255, 0, 1);

    mrc_refreshScreen(0, 0, 240, 320);
    return (uint32)dsm_init;
}

int32 mrc_exitApp(void) {
    return MR_SUCCESS;
}

int32 mrc_event(int32 code, int32 param0, int32 param1) {
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
