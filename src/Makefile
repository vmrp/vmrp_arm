
MINI_VM_VERSION := 2011
VM_VERSION := 1968
MINI_TARGETDIR :=

LOCAL_CFLAGS_FULL := -g -Wall -c -littleend -DDSM_FULL -DTARGET_MOD -DMTK_MOD -DMR_VERSION=$(VM_VERSION) -DMR_PLAT_DRAWTEXT

LOCAL_SRC_FILES_FULL := src/mr_api.c \
                  src/mr_debug.c  \
                  src/mr_do.c     \
                  src/mr_dump.c   \
                  src/mr_func.c     \
                  src/mr_gc.c       \
                  src/mr_mem.c      \
                  src/mr_opcodes.c  \
                  src/mr_object.c  \
                  src/mr_state.c    \
                  src/mr_string.c   \
                  src/mr_table.c    \
                  src/mr_tm.c       \
                  src/mr_undump.c   \
                  src/mr_vm.c       \
                  src/mr_zio.c      \
                  src/mr_noparser.c  
#                 src/lparser.c   \
#                 src/mr_cect.c   \
#                 src/llex.c      \
#                 src/lcode.c   \

LOCAL_SRC_FILES_FULL += src/lib/mr_auxlib.c \
                  src/lib/mr_auxiliar.c\
                  src/lib/mr_baselib.c\
                  src/lib/mr_tablib.c   \
                  src/lib/mr_strlib.c     \
                  src/lib/mr_socket_target.c     \
                  src/lib/mr_tcp_target.c     \
                  src/lib/mr_iolib_target.c    
#                  src/loadlib.c     \
#                  src/ldblib.c        \

LOCAL_SRC_FILES_FULL += mythroad.c \
                    mr_gb2312.c     \
                    mr_pluto.c	\
                    mr_unzip.c	\
                    mr_encode.c	\
		    		mr_graphics.c \
                    mr_inflate.c \
                    string.c	\
                    printf.c	\
                    other.c	\
                    strtol.c	\
                    strtoul.c	\
                    dsm.c	\
                    fixR9.c	\
                    md5.c	\
                    mem.c	\
                    asm/r9r10.s	\
#                    src/mr_maketo.c	
#                    src/MySprite.c   \
#                    src/pluto.c	\
#                    src/Mylib.c

LOCAL_SRC_FILES_FULL += tomr/tomr_event.c \
                tomr/tomr_is.c   \
                tomr/tomr_map.c        \
                tomr/tomr_push.c     \
                tomr/tomr_to.c

full:
	gcc $(LOCAL_CFLAGS_FULL) $(LOCAL_SRC_FILES_FULL) -I./src -I./tomr -I./include
	# rm ../libmr_vm.a
	ar crv ../libmr_vm.a *.o
	rm *.o

#####################################################################

LOCAL_CFLAGS :=  -fPIC -march=armv5te -marm -g -Wall -c -littleend  -DTARGET_MOD -DMTK_MOD -DMR_VERSION=$(MINI_VM_VERSION) -DMR_PLAT_DRAWTEXT
LOCAL_SRC_FILES := mythroad_mini.c \
                    mr_gb2312.c     \
                    mr_unzip.c	\
                    mr_encode.c	\
                    mr_inflate.c \
                    mr_graphics.c \
                    string.c \
                    printf.c	\
                    other.c	\
                    strtol.c	\
                    strtoul.c	\
                    dsm.c	\
                    fixR9.c	\
                    md5.c	\
                    mem.c	\
                    asm/r9r10.s	\

mini:
	gcc $(LOCAL_CFLAGS) $(LOCAL_SRC_FILES) -I./include -I.
	# rm ../libmr_vm.a
	ar crv ../libmr_vm.a *.o
	rm *.o


.PHONY: clean
clean:
	rm *.a *.o
