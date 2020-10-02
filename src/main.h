#ifndef _EMULATOR_H
#define _EMULATOR_H

#include "./mr/include/mr_helper.h"
#include "./mr/include/type.h"
#include "dsm.h"


#define panic(msg)           \
    do {                     \
        printf("%s\n", msg); \
        while (1) {          \
        }                    \
    } while (0)

int getFileType(const char *name);
int getFileSize(const char *path);
void logPrint(char *level, char *tag, ...);

int64 get_time_ms(void);

void emu_bitmapToscreen(uint16 *data, int x, int y, int w, int h);
int32 emu_timerStart(uint16 t);
int32 emu_timerStop(void);
void emu_finish();

extern int32 mr_start_dsmC(char *start_file, const char *entry);

#endif
