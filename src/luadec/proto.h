#ifndef PROTO_H
#define PROTO_H

#ifndef LUA_OPNAMES
#define LUA_OPNAMES
#endif

#include "../src/h/mr_debug.h"
#include "../src/h/mr_object.h"
#include "../src/h/mr_opcodes.h"
#include "../src/h/mr_undump.h"

char *DecompileString(const Proto * f, int n);

char *DecompileConstant(const Proto * f, int i);

#endif
