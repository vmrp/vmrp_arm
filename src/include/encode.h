#ifndef _MR_ENCODE_H_
#define _MR_ENCODE_H_

#include "type.h"

uint16 *c2u(const char *cp, int *err, int *size);

uint16 GBCharToUCS2BEChar(uint8 *gbCode);
uint16 UCS2LECharToGBChar(uint16 ucs);

// 如果传了outMemLen参数，则释放内存时需要用带len参数的mr_free()释放内存
// 如果outMemLen传NULL，则用mr_freeExt()释放内存

uint16 *GBToUCS2BEStr(uint8 *gbCode, uint32 *outMemLen);
char *UCS2BEStrToGBStr(uint16 *uniStr, uint32 *outMemLen);
char *UTF8ToGBString(uint8 *str, uint32 *outMemLen);
char *UCS2BEToUTF8(const uint8 *unicode, uint32 *outMemLen);

#endif
