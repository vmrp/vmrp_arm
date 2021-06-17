#include "../include/dsm.h"

int32 mrc_init(void) {
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
