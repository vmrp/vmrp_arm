#include "other.h"

#include <stdlib.h>

unsigned long strtoul2(const char *s, char **end_ptr, int base) {
    return strtoul(s, end_ptr, base);
}

long int strtol2(const char *str, char **endptr, int base) {
    return strtol(str, endptr, base);
}

int rand2(void) {
    return rand();
}

static long atol2(const char *s) {
    unsigned long ret = 0;
    unsigned long d;
    int neg = 0;

    if (*s == '-') {
        neg = 1;
        s++;
    }

    while (1) {
        d = (*s++) - '0';
        if (d > 9)
            break;
        ret *= 10;
        ret += d;
    }

    return neg ? -ret : ret;
}

int atoi2(const char *s) {
    return atol2(s);
}
///////////////////////////////////////////////////////////////////////////

int isxdigit(int ch) {
    return ((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'f')) || ((ch >= 'A') && (ch <= 'F'));
}

int isalpha(int ch) {
    return ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z'));
}

int isspace(int ch) {
    return (ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n') || (ch == '\f') || (ch == '\v');
}

int toupper(int ch) {
    if ((unsigned int)(ch - 'a') < 26u)
        ch += 'A' - 'a';
    return ch;
}

int tolower(int ch) {
    if ((unsigned int)(ch - 'A') < 26u)
        ch += 'a' - 'A';
    return ch;
}

int isalnum(int ch) {
    return (unsigned int)((ch | 0x20) - 'a') < 26u || (unsigned int)(ch - '0') < 10u;
}

int isupper(int ch) {
    return (unsigned int)(ch - 'A') < 26u;
}

int isprint(int ch) {
    return (unsigned int)(ch - ' ') < 127u - ' ';
}

int ispunct(int ch) {
    return isprint(ch) && !isalnum(ch) && !isspace(ch);
}