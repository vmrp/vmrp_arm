.PHONY: vmrp
vmrp:
	gcc -o $@ -g -fPIE  -march=armv5te -marm -Wall main.c -L. -lmr_vm -lSDL2 -lm -lz 

.PHONY: clean
clean:
	rm *.a *.o


# -mthumb
# -marm
# -msingle-pic-base -mpic-register=r9

# -mpic-data-is-text-relative
# -mword-relocations


# -fpic  -fPIC  -fpie  -fPIE  -fno-plt  -static-pie
# https://gcc.gnu.org/onlinedocs/gcc-8.4.0/gcc/ARM-Options.html#ARM-Options
# https://gcc.gnu.org/onlinedocs/gcc-8.4.0/gcc/Code-Gen-Options.html#Code-Gen-Options
# https://gcc.gnu.org/onlinedocs/gcc-8.4.0/gcc/Option-Summary.html#Option-Summary
 
 