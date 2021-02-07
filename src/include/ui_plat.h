#ifndef UI_PLAT_H
#define UI_PLAT_H


#include "mem.h"
#include "other.h"
#include "dsm.h"
int32 mr_dialogCreate(const char *title, const char *text, int32 type);
int32 mr_dialogRelease(int32 id);
int32 mr_dialogRefresh(int32 dialog, const char *title, const char *text, int32 type);
int32 mr_editCreate(const char *title, const char *text, int32 type, int32 max_size);
int32 mr_editRelease(int32 id);
const char *mr_editGetText(int32 edit);



int32 ui_event(int32 type,int32 p1,int32 p2);


#endif
