
LOCAL_SRC_FILES := mythroad.c \
	mr_draw.c \
	mr_file.c \
	mr_mem.c \
	mr_smscfg.c \
	mr_utils.c

mythroad:
	gcc -g -Wall -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-stringop-truncation\
             -c   $(LOCAL_SRC_FILES)\
                -I.

	ar crv ./libmythroad.a *.o


.PHONY: clean
clean:
	rm *.a *.o