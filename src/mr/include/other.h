#ifndef _OTHER_H_
#define _OTHER_H_

#include <stdarg.h>
#include <setjmp.h>



int atoi2(const char* s);
int rand2(void);
unsigned long strtoul2(const char* s, char** end_ptr, int base);
long int strtol2(const char* str, char** endptr, int base);

#endif
