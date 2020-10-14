.PHONY: vmrp
vmrp:
	gcc -o $@ -g -Wall main.c -L. -lmr_vm -lSDL2 -lm -lz 

.PHONY: clean
clean:
	rm *.a *.o