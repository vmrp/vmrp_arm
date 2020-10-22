
/*
r9寄存器又叫sb寄存器（静态基址寄存器），是ext内全局变量的基地址

BUG原因是ext中mr_helper()会修改r9寄存器的值，而调用mythroad中的函数时并没有恢复r9的值
目前仅对mr_c_function_load(0);初始化方式进行修复，传其它参数调用mr_c_function_load()的结果尚未研究

别名：
ext中的 mr_helper() 在mythroad中叫 mr_c_function()
ext的第一个函数是ext文件起始位置+8叫 mr_c_function_load() ,在mythroad中叫 mr_load_c_function()


mr_c_function_load()会回调mythroad中的_mr_c_function_new()把mr_helper()函数地址传回mythroad
此时_mr_c_function_new()会设置mr_c_function_P

_mr_c_function_new() 返回后再由ext设置mr_c_function_P的内容:
mr_c_function_P.ER_RW_Length(uint32类型偏移量4) = mr_helper_get_rw_len();
mr_c_function_P.start_of_ER_RW(void*类型偏移量0) =  mrc_malloc(mr_c_function_P.ER_RW_Length); //注意mrc_malloc()是ext内的

因为ext里所有的事件函数都是通过 mythroad调用 mr_helper() 分发
而 mr_helper() 进去后就会将r9寄存器的值备份到r10并设置为 mr_c_function_P.start_of_ER_RW 的值， 直到返回才会 mov r10,r9 恢复原来的r9值
之后在ext空间内调用mythroad层的函数时因为r9寄存的值没有恢复，导致mythroad层全局变量的引用全部跑飞


ext内的mrc_malloc和mrc_free经过如下包装
void *mrc_malloc(uint32 len) {
    uint32 *p = _mr_c_function_table.mr_malloc(len + sizeof(uint32));
    if (p) {
        *p = len;
        return (void *)(p + 1);
    }
    return p;
}

void mrc_free(void *p) {
    uint32 *t = (uint32 *)p - 1;
    _mr_c_function_table.mr_free(t, *t);
}

解决办法：
因为r9始终为mr_c_function_P.start_of_ER_RW，所以在mythroad空间也能够使用，并且貌似是唯一可以使用的值
所以需要借助r9来恢复r9和r10

要求在进入 mr_helper() 前通过mr_c_function_P.start_of_ER_RW备份 r9和r10
ext调用mythroad时在mythroad空间通过r9 恢复 r9 r10

因为ext中始终都是以mr_c_function_P.start_of_ER_RW为基址向上访问，因此我们重新分配它，向下访问将能达到我们的目的
在mr_c_function_load()之后重新对此内存进行分配

20201021上面的方案证实在非插件化的mrp中有效，但是插件化mrp因为又套了一层ext，所以r9r10又失效了，mmp
套娃的ext我们不知道它在何时设置mr_c_function_P.start_of_ER_RW，也没办法在它设置之后实施上面的方案，如果它又套一层呢？
因为mr_c_function_P.start_of_ER_RW也是用malloc获取的内存，因此在所有malloc上都加上我们的数据，效率可能会低，没办法了

*/

#include "./include/fixR9.h"

// 加上4是因为ext的内存申请是通过mrc_malloc()，而mrc_malloc()包装过返回的是实际地址+4的值
#define CTX_POS (4 + sizeof(fixR9_st))
typedef struct fixR9_st {
    void *r9Mythroad;
    void *r10Mythroad;
    void *rwMem1;
    void *rwMem2;
} fixR9_st;

static void *r9Mythroad;
static void *r10Mythroad;
static void *lr;

extern void *mr_malloc(uint32 len);
extern void mr_free(void *p, uint32 len);
extern void *mr_realloc(void *p, uint32 oldlen, uint32 len);

void *mr_malloc_ext(uint32 len) {
    char *mem;
    fixR9_st *ctx;
    len += sizeof(fixR9_st);
    mem = mr_malloc(len);
    ctx = (fixR9_st *)mem;
    ctx->r10Mythroad = r10Mythroad;
    ctx->r9Mythroad = r9Mythroad;
    ctx->rwMem1 = mem + CTX_POS;
    ctx->rwMem2 = ctx->rwMem1;
    return (char *)mem + sizeof(fixR9_st);
}

void mr_free_ext(void *p, uint32 len) {
    len += sizeof(fixR9_st);
    mr_free((char *)p - sizeof(fixR9_st), len);
}

void *mr_realloc_ext(void *p, uint32 oldlen, uint32 len) {
    oldlen += sizeof(fixR9_st);
    len += sizeof(fixR9_st);
    return mr_realloc((char *)p - sizeof(fixR9_st), oldlen, len);
}

void fixR9_saveLR(void *v) {
    lr = v;
}

void *fixR9_getLR() {
    return lr;
}

#ifndef __GNUC__

static BOOL isInExt;
static void *r9Ext;
static void *r10Ext;

void fixR9_saveMythroad() {
    r9Mythroad = getR9();
    r10Mythroad = getR10();
}

void fixR9_begin() {  // 注意，这里可能在ext空间执行，不能直接使用context
    void *r9v = getR9();
    void *r10v = getR10();
    if ((uint32)r9v > CTX_POS) {
        // todo 注意，因为r9的值不确实，所以ctx有可能会是个无效的内存地址，导致程序崩溃，目前还不知道怎样获得有效地址的范围
        fixR9_st *ctx = (fixR9_st *)((char *)r9v - CTX_POS);
        if (ctx && (r9v == ctx->rwMem1) && (r9v == ctx->rwMem2)) {  // 是在ext空间
            setR9R10(ctx->r9Mythroad, ctx->r10Mythroad);
            r9Ext = r9v;
            r10Ext = r10v;
            isInExt = TRUE;
        }
    }
}

void fixR9_end() {
    if (isInExt) {
        setR9R10(r9Ext, r10Ext);
    }
}

#else

void *getR9() {
    register void *ret;
    asm("MOV %[result], r9"
        : [ result ] "=r"(ret));
    return ret;
}

void setR9(void *value) {
    asm("MOV r9, %[input_value]"
        :
        : [ input_value ] "r"(value));
}

void *getR10() {
    register void *ret;
    asm("MOV %[result], r10"
        : [ result ] "=r"(ret));
    return ret;
}

void setR10(void *value) {
    asm("MOV r10, %[input_value]"
        :
        : [ input_value ] "r"(value));
}
#endif
