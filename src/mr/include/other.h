#ifndef _OTHER_H_
#define _OTHER_H_

#include <stdarg.h>
#include <setjmp.h>



int atoi2(const char* s);
int rand2(void);
long strtol2(const char *nptr, char **endptr, register int base);
unsigned long strtoul2(const char *nptr, char **endptr, register int base);

#endif
