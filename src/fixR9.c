
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




*/

#include "./include/fixR9.h"

#define MRDBGPRINTF mr_printf

extern void *mr_malloc(uint32 len);
extern void mr_free(void *p, uint32 len);

void *mr_malloc2(uint32 len) {
    uint32 *p = mr_malloc(len + sizeof(uint32));
    if (p) {
        *p = len;
        return (void *)(p + 1);
    }
    return p;
}

void mr_free2(void *p) {
    if (p) {
        uint32 *t = (uint32 *)p - 1;
        mr_free(t, *t);
    }
}

typedef struct fixR9_st {
    BOOL isInMythroad;
    void *r9Mythroad;
    void *r10Mythroad;
    void *r9Ext;
    void *r10Ext;
    void *rwMemCheck;
    void *rwMem;
    uint32 rwLen;  // 保持在最后一个
} fixR9_st;

static fixR9_st *context;

int32 fixR9_init() {
    context = NULL;
    return MR_SUCCESS;
}

int32 fixR9_hack(mr_c_function_P_st *mr_c_function_P) {
    char *mem, *newRw;

    MRDBGPRINTF("fixR9_init-----------------start_of_ER_RW:0x%X, len:0x%X", mr_c_function_P->start_of_ER_RW, mr_c_function_P->ER_RW_Length);
    if (context) {
        return MR_FAILED;
    }

    mem = mr_malloc2(mr_c_function_P->ER_RW_Length + sizeof(fixR9_st));
    if (!mem) {
        return MR_FAILED;
    }
    newRw = mem + sizeof(fixR9_st);
    memcpy2(newRw, mr_c_function_P->start_of_ER_RW, mr_c_function_P->ER_RW_Length);
    mr_free2(mr_c_function_P->start_of_ER_RW);

    // 不用管mr_c_function_P->ER_RW_Length
    mr_c_function_P->start_of_ER_RW = newRw;

    context = (fixR9_st *)mem;
    context->rwMemCheck = (uint32 *)mr_c_function_P->start_of_ER_RW - 1;
    context->rwMem = mr_c_function_P->start_of_ER_RW;
    context->rwLen = mr_c_function_P->ER_RW_Length;
    context->isInMythroad = TRUE;
    // MRDBGPRINTF("test:---%X %X", context->rwMemCheck, &context->rwLen); // 两个值应该是相等的
    return MR_SUCCESS;
}

// 检测是否对mr_c_function_P->start_of_ER_RW进行了free()操作，目前还不知道什么时候会free()，也许并不会在ext中free()
BOOL fixR9_checkFree(void *p) {
    return (context && (p == context->rwMemCheck));
}

#ifdef __GNUC__
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

void fixR9_save() {
    if (context) {
        context->r9Mythroad = getR9();
        context->r10Mythroad = getR10();
    }
}

void fixR9_setIsInMythroad(BOOL v) {
    if (context) {
        context->isInMythroad = v;
    }
}

static BOOL isInExt(void *r9v) {
    if ((uint32)r9v > sizeof(fixR9_st)) {
        fixR9_st *ctx = (fixR9_st *)((char *)r9v - sizeof(fixR9_st));
        if (ctx && (r9v == ctx->rwMem)) {      // todo 注意，ctx有可能会是个无效的内存地址，目前还不知道怎样获得有效地址的范围
            if (ctx->isInMythroad == FALSE) {  // 加多一层，避免误判
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*
todo 如果mythroad函数的参数大于4个，可能会在调用之前使用了r10，
而此函数需要在目标函数的入口处第一时间执行，相当于还在ext空间
如果遇到这种情况直接执行此函数将会导致mythroad空间的r10再次被破坏
对于这样的函数需要用汇编才能解决
*/
void fixR9_begin() {
    void *r9v = getR9();
    void *r10v = getR10();
    if (isInExt(r9v)) {
        fixR9_st *ctx = (fixR9_st *)((char *)r9v - sizeof(fixR9_st));
        void *newR9v = ctx->r9Mythroad;  // 必需先用寄存器保存
        void *newR10v = ctx->r10Mythroad;
        // 注意，这里在ext空间，不能直接使用context
        ctx->r9Ext = r9v;
        ctx->r10Ext = r10v;
        setR9R10(newR9v, newR10v);
    }
}

void fixR9_end() {
    if (context && (context->isInMythroad == FALSE)) {
        void *newR9v = context->r9Ext;  // 必需先用寄存器保存
        void *newR10v = context->r10Ext;
        setR9R10(newR9v, newR10v);
    }
}