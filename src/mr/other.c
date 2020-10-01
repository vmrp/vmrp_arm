#include "other.h"

#include <stdlib.h>


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



