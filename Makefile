#编译完整虚拟机
mrpoid2:
	gcc -o $@ -g -Wall -DDSM_FULL main.c  -L. -lmr_vm_full -lSDL2 -lm -lz 


#编译精简虚拟机
# mrpoid:
# 	gcc -o $@ -O2 -g -Wall -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-unused-variable -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-stringop-truncation -Wno-implicit-function-declaration\
# 		-Wno-unused-function -Wno-uninitialized \
# 		-DDSM_MINI \
# 		main.c \
# 		dsm.c \
# 		network.c \
# 		utils.c \
# 		encode.c \
# 		font_sky16_2.c \
# 		-L./mr -lmr_vm_mini -lSDL2 -lm -lz \




.PHONY: clean
clean:
	rm *.a *.o