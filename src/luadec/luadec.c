
#include "../include/mr.h"
#include "../include/mr_auxlib.h"
#include "../src/h/mr_func.h"
#include "../src/h/mr_mem.h"
#include "../src/h/mr_object.h"
#include "../src/h/mr_opcodes.h"
#include "../src/h/mr_string.h"
#include "../src/h/mr_undump.h"
#include "print.h"

#ifndef MRP_DEBUG
#define mr_B_opentests(L)
#endif

static int debugging = 0; /* debug decompiler? */
static int functions = 0; /* dump functions separately? */
// static int dumping = 1;   /* dump bytecodes? */
// static int stripping = 0; /* strip debug information? */


static Proto* toproto(mrp_State* L, int i) {
    const Closure* c = (const Closure*)mrp_topointer(L, i);
    return c->l.p;
}

static Proto* combine(mrp_State* L, int n) {
    if (n == 1)
        return toproto(L, -1);
    else {
        int i, pc = 0;
        Proto* f = mr_F_newproto(L);
        f->source = mr_S_newliteral(L, "=(luadec)");
        f->maxstacksize = 1;
        f->p = mr_M_newvector(L, n, Proto*);
        f->sizep = n;
        f->sizecode = 2 * n + 1;
        f->code = mr_M_newvector(L, f->sizecode, Instruction);
        for (i = 0; i < n; i++) {
            f->p[i] = toproto(L, i - n);
            f->code[pc++] = CREATE_ABx(OP_CLOSURE, 0, i);
            f->code[pc++] = CREATE_ABC(OP_CALL, 0, 1, 1);
        }
        f->code[pc++] = CREATE_ABC(OP_RETURN, 0, 1, 0);
        return f;
    }
}

/*static*/ void strip(mrp_State* L, Proto* f) {
    int i, n = f->sizep;
    mr_M_freearray(L, f->lineinfo, f->sizelineinfo, int);
    mr_M_freearray(L, f->locvars, f->sizelocvars, struct LocVar);
    mr_M_freearray(L, f->upvalues, f->sizeupvalues, TString*);
    f->lineinfo = NULL;
    f->sizelineinfo = 0;
    f->locvars = NULL;
    f->sizelocvars = 0;
    f->upvalues = NULL;
    f->sizeupvalues = 0;
    f->source = mr_S_newliteral(L, "=(none)");
    for (i = 0; i < n; i++) strip(L, f->p[i]);
}

int luadec(mrp_State* L, char* filename, char* outputFile) {
    Proto* f;
    // mrp_State* L = mrp_open();
    mr_B_opentests(L);

    // 此处可以load多个
    if (mr_L_loadfile(L, filename) != 0) {
        mr_printf("luadec: %s", mrp_tostring(L, -1));
        return -1;
    }

    f = combine(L, 1);  // 只有一个文件
    if (functions)
        luaU_decompileFunctions(f, debugging, outputFile);
    else
        luaU_decompile(f, debugging, outputFile);
    // mrp_close(L);
    mr_printf("luadec done: %s -> %s", filename, outputFile);
    return 0;
}
