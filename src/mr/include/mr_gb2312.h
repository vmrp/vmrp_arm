
#ifndef __MR_GB2312_H_
#define __MR_GB2312_H_

typedef  unsigned short unicode_char;

extern unicode_char *c2u(const char *cp, int *err, int *size);

#endif
