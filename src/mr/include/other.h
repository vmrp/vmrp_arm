#ifndef _OTHER_H_
#define _OTHER_H_

#include <stdarg.h>
#include <setjmp.h>

#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define islower(ch) ((ch) >= 'a') && ((ch) <= 'z')
#define iscntrl(ch) ((unsigned int)(ch) < 32u || (ch) == 127)

int isxdigit(int ch);
int isalpha(int ch);
int isspace(int ch);
int toupper(int ch);
int tolower(int ch);
int isalnum(int ch);
int isupper(int ch);
int isprint(int ch);
int ispunct(int ch);

int atoi2(const char* s);
int rand2(void);
unsigned long strtoul2(const char* s, char** end_ptr, int base);
long int strtol2(const char* str, char** endptr, int base);

#endif
