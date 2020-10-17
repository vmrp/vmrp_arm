#ifndef __Fix_R9__
#define __Fix_R9__

#include "mr.h"
#include "type.h"

// 从ext反汇编而来，并不是完整的定义，为了使用方便只定义了已知用途的字段
typedef struct {
    void *start_of_ER_RW;
    uint32 ER_RW_Length;
} mr_c_function_P_st;

int32 fixR9_init(void);
int32 fixR9_hack(mr_c_function_P_st *mr_c_function_P);
void fixR9_save(void);
void fixR9_setIsInMythroad(BOOL v);
BOOL fixR9_checkFree(void *p);

int32 fixR9_begin(void);
int32 fixR9_end(void);

extern void *getR9(void);
extern void setR9(void *value);
extern void *getR10(void);
extern void setR10(void *value);

#endif
