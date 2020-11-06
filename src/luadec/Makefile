# makefile for Lua decompiler
# gcc -g -Wall -I../../include -I..    -g   -c -o luadec.o luadec.c
# gcc -g -Wall -I../../include -I..    -g   -c -o print.o print.c
# gcc -o lopcodes.o -c -g -Wall -I../../include -I..    -g -DLUA_OPNAMES ../lopcodes.c
# gcc -g -Wall -I../../include -I..    -g   -c -o proto.o proto.c
# gcc -g -Wall -I../../include -I..    -g   -c -o StringBuffer.o StringBuffer.c
# gcc -g -Wall -I../../include -I..    -g   -c -o structs.o structs.c
# gcc -o ../../bin/luadec  luadec.o print.o lopcodes.o proto.o StringBuffer.o structs.o -L../../lib -llua -llualib -lm


LUA= ../..

include $(LUA)/config

CFLAGS += -g

INCS= -I$(INC) -I.. $(EXTRA_INCS)
OBJS= luadec.o print.o lopcodes.o proto.o StringBuffer.o structs.o
SRCS= luadec.c print.c proto.c StringBuffer.c structs.c

T= $(BIN)/luadec

all:	$T

$T:	$(OBJS) $(LIB)/liblua.a $(LIB)/liblualib.a
	$(CC) -o $@ $(MYLDFLAGS) $(OBJS) -L$(LIB) -llua -llualib $(EXTRA_LIBS) $(DLLIB)

# print.c needs opcode names from lopcodes.c
lopcodes.o:	../lopcodes.c ../lopcodes.h
	$(CC) -o $@ -c $(CFLAGS) -DLUA_OPNAMES ../lopcodes.c

$(LIB)/liblua.a:
	cd ..; $(MAKE)

$(LIB)/liblualib.a:
	cd ../lib; $(MAKE)

clean:
	rm -f $(OBJS) $T

co:
	co -q -f -M $(SRCS)

klean:	clean
	rm -f $(SRCS)
