
typedef struct funcTable {
    void (*log)(char *msg);
} funcTable;

typedef struct cbTable {
    int (*test)(void);
} cbTable;

static funcTable *funcs;
static cbTable cbs;

int test(void) {
    funcs->log("hello mrp!");
    return 123;
}

cbTable *start(funcTable *p) {
    funcs = p;
    cbs.test = test;
    return &cbs;
}

#include "mrc_base.h"
int32 mrc_init(void) {
    mrc_drawText("init.", 0, 40, 255, 255, 255, 0, 1);
    return (int32)start;
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
