
#include "./include/encode.h"

#include "./include/mem.h"
#include "./include/tables.h"
// ucs-2be与utf-16be基本是一样的，ucs2是固定一个字符两字节，而utf-16一个字符最大可以有4字节

#ifdef USE_VM_C2U
uint16 *c2u(const char *cp, int *err, int *size) {
    size_t i, cnt;
    uint16 *uc;

    if (err) *err = -1;

    // Count the number of potential unicode characters first.
    for (i = cnt = 0; cp[i]; i++) {
        if ((int)(unsigned char)cp[i] < 0xA1 ||
            (int)(unsigned char)cp[i] > 0xFE ||
            cp[i + 1] == 0) {
            ++cnt;
            continue;
        }
        ++i, ++cnt;
    }

    *size = (cnt + 1) * sizeof(uint16);
    uc = (uint16 *)mr_malloc(*size);
    if (!uc) return (NULL);

    i = cnt = 0;
    while (cp[i]) {
        int a = (int)(unsigned char)cp[i];
        int b = (int)(unsigned char)cp[i + 1];

        if (a >= 0xA1 && a <= 0xFE && b) {
            uint16 ucv;

            if (0xA1 <= b && b <= 0xFE && mr_gb2312[a - 0xA1] && (ucv = mr_gb2312[a - 0xA1][b - 0xA1])) {
                uc[cnt++] = (ucv << 8) | (ucv >> 8);
            } else if (err) {
                *err = i;
                mr_free(uc, *size);
                return NULL;
            } else {
                //uc[cnt++] = (uint16)0xFFFD;
                uc[cnt++] = 0xFDFF;
            }
            i += 2;
        } else if (a < (unsigned)0x80) {
            uc[cnt++] = a << 8;
            i += 1;
        } else if (err) {
            //  uc[cnt++]= 0xa025;//(ucv << 8) +  (ucv >> 8);
            //  i += 2;
            *err = i;
            mr_free(uc, *size);
            return (NULL);
        } else {
            //uc[cnt++] = (uint16)0xFFFD;
            uc[cnt++] = 0xFDFF;
            i += 2;
        }
    }
    uc[cnt] = 0;
    return (uc);
}
#else

uint16 GBCharToUCS2BEChar(uint8 *gbCode) {
    const uint8 *p_map = NULL;
    int i;
    if ((*(gbCode + 1) >= 0xa1) && (*(gbCode + 1) <= 0xfe)) {
        if ((*gbCode >= 0xa1) && (*gbCode <= 0xa9)) {
            i = ((*gbCode - 0xa1) * 94 + (*(gbCode + 1) - 0xa1)) * 2;
            if (i < GB2UTABLE_LEN) {
                p_map = &gb2uTable[i];
            }
        } else if ((*gbCode >= 0xb0) && (*gbCode <= 0xf7)) {
            i = ((*gbCode - 0xb0 + 9) * 94 + (*(gbCode + 1) - 0xa1)) * 2;
            if (i < GB2UTABLE_LEN) {
                p_map = &gb2uTable[i];
            }
        }
    }
    if (p_map == NULL) {
        return 0xFDFF;  // '�'
        // return 0x0030;  // 大端全角空格
    }
    return (p_map[1]) | (p_map[0] << 8);
}

// 如果传了outMemLen参数，则必需用带len参数的free释放内存
uint16 *GBStrToUCS2BEStr(uint8 *gbCode, uint32 *outMemLen) {
    uint32 i = 0, j = 0, len;
    uint16 *unicode;

    // 原c2u允许""空字符串转换
    if (!gbCode) return NULL;

    while (gbCode[i]) {
        if (gbCode[i] < 0x80) {
            i += 1;
        } else {
            i += 2;
        }
        j++;
    }
    len = (j + 1) * sizeof(uint16);
    if (outMemLen) {
        unicode = mr_malloc(len);
        *outMemLen = len;
    } else {
        unicode = mr_mallocExt(len);
    }
    if (!unicode) return NULL;
    i = j = 0;
    while (gbCode[i]) {
        if (gbCode[i] < 0x80) {
            unicode[j] = gbCode[i] << 8;
            i += 1;
        } else {
            unicode[j] = GBCharToUCS2BEChar(&gbCode[i]);
            i += 2;
        }
        j++;
    }
    unicode[j] = 0;
    return unicode;
}

uint16 *c2u(const char *cp, int *err, int *size) {
    if (err) *err = -1;
    return GBStrToUCS2BEStr((uint8 *)cp, (uint32 *)size);
}

#endif

uint16 UCS2LECharToGBChar(uint16 ucs) {
    int First = 0;
    int Last = TAB_SYMUCS2GB_LEN - 1;
    int Mid, i;

    if (ucs >= 0x4E00 && ucs <= 0x9FA5) {
        i = ucs - 0x4E00;
        if (i < TAB_UCS2GB_LEN) {
            return tab_ucs2gb[i];
        }
    } else {
        while (Last >= First) {
            Mid = (First + Last) >> 1;
            if (ucs < tab_symucs2gb[Mid].ucs) {
                Last = Mid - 1;
            } else if (ucs > tab_symucs2gb[Mid].ucs) {
                First = Mid + 1;
            } else if (ucs == tab_symucs2gb[Mid].ucs) {
                return tab_symucs2gb[Mid].gb;
            }
        }
    }
    return 0xffff;
}

// 如果传了outMemLen参数，则必需用带len参数的free释放内存
char *UCS2BEStrToGBStr(uint16 *uniStr, uint32 *outMemLen) {
    uint32 len = 1, i = 0;
    uint16 *p = uniStr;
    uint8 *gb;

    while (*p) {
        uint16 tmp = (*p << 8) | (*p >> 8);
        if (tmp < 0x80) {
            len += 1;
        } else {
            len += 2;
        }
        p++;
    }
    if (outMemLen) {
        gb = mr_malloc(len);
        *outMemLen = len;
    } else {
        gb = mr_mallocExt(len);
    }
    if (!gb) return NULL;

    p = uniStr;
    while (*p) {
        uint16 tmp = (*p << 8) | (*p >> 8);
        if (tmp < 0x80) {
            gb[i++] = (uint8)tmp;
        } else {
            uint16 Gb = UCS2LECharToGBChar(tmp);
            gb[i++] = (uint8)(Gb >> 8);
            gb[i++] = (uint8)(Gb & 0xff);
        }
        p++;
    }
    gb[i] = '\0';
    return (char *)gb;
}

char *UTF8StrToGBStr(uint8 *str, uint32 *outMemLen) {
    uint32 len = 1;
    uint16 c;
    uint8 *utf8Str, *gb, *mem;

    utf8Str = str;
    while (*utf8Str) {
        if (*utf8Str < 0x80) {  // 1 Byte
            len += 1;
            utf8Str += 1;
            continue;
        } else if ((*utf8Str & 0xe0) == 0xc0) {  // 2 Bytes
            utf8Str += 2;
        } else if ((*utf8Str & 0xf0) == 0xe0) {  // 3 Bytes
            utf8Str += 3;
        } else {
            break;
        }
        len += 2;
    }
    if (outMemLen) {
        mem = mr_malloc(len);
        *outMemLen = len;
    } else {
        mem = mr_mallocExt(len);
    }
    gb = mem;
    if (!gb) return NULL;
    utf8Str = str;
    while (*utf8Str) {
        if (*utf8Str < 0x80) {  // 1 Byte
            *gb++ = *utf8Str++;
            continue;
        } else if ((*utf8Str & 0xe0) == 0xc0) {  // 2 Bytes
            c = UCS2LECharToGBChar(((utf8Str[0] & 0x1f) << 6) | (utf8Str[1] & 0x3f));
            utf8Str += 2;
        } else if ((*utf8Str & 0xf0) == 0xe0) {  // 3 Bytes
            c = UCS2LECharToGBChar(((utf8Str[0] & 0x0f) << 12) | ((utf8Str[1] & 0x3f) << 6) | ((utf8Str[2] & 0x3f)));
            utf8Str += 3;
        } else {
            break;
        }
        *gb++ = (uint8)(c >> 8);
        *gb++ = (uint8)(c & 0xff);
    }
    *gb = '\0';
    return (char *)mem;
}

// 如果传了outMemLen参数，则必需用带len参数的free释放内存
char *UCS2BEStrToUTF8Str(const uint8 *unicode, uint32 *outMemLen) {
    int u = 0, i = 0, len = 1;
    char *utf8;
    while ((unicode[u] || unicode[u + 1])) {
        if (unicode[u] == 0 && unicode[u + 1] < 0x80) {  // 0 - 7 bits
            len += 1;
        } else if ((unicode[u] & 0xf8) == 0) {  // 8 - 11 bits
            len += 2;
        } else {  // 12 - 16 bits
            len += 3;
        }
        u += 2;
    }
    if (outMemLen) {
        utf8 = mr_malloc(len);
        *outMemLen = len;
    } else {
        utf8 = mr_mallocExt(len);
    }
    if (!utf8) return NULL;
    u = 0;
    while ((unicode[u] || unicode[u + 1])) {
        if (unicode[u] == 0 && unicode[u + 1] < 0x80) {  // 0 - 7 bits
            utf8[i++] = unicode[u + 1];
        } else if ((unicode[u] & 0xf8) == 0) {  // 8 - 11 bits
            utf8[i++] = 0xc0 | (unicode[u] << 2) | (unicode[u + 1] >> 6);
            utf8[i++] = 0x80 | (unicode[u + 1] & 0x3f);
        } else {  // 12 - 16 bits
            utf8[i++] = 0xe0 | (unicode[u] >> 4);
            utf8[i++] = 0x80 | ((unicode[u] & 0x0f) << 2) | (unicode[u + 1] >> 6);
            utf8[i++] = 0x80 | (unicode[u + 1] & 0x3f);
        }
        u += 2;
    }
    utf8[i] = '\0';
    return utf8;
}

/*
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int getLen(const char *filename) {
    struct stat s1;

    int ret = stat(filename, &s1);
    if (ret != 0)
        return -1;
    return s1.st_size;
}

const char *ustr = "\x6d\x4b\x8b\xd5\x00\x74\x00\x65\x00\x73\x00\x74\x00\x00";  //"测试test"

int main(int argc, char const *argv[]) {
    char *filename = "utf8.txt";
    int len = getLen(filename);
    int f = open(filename, O_CREAT | O_RDWR, 0666);
    char *buf = malloc(len);
    if (read(f, buf, len) != len) {
        printf("readlen != len\n");
    } else {
        int size;
        // uint16 *strBuf = c2u(buf, NULL, &size);
        char *strBuf = UTF8ToGBString(buf, &size);
        // char *strBuf = UCS2BEToUTF8((uint8 *)ustr, &size);
        // char *strBuf = UCS2BEToGB((uint16 *)ustr, &size);
        printf("%s\n", strBuf);

        if (strBuf != NULL) {
            int outf = open("out.txt", O_CREAT | O_RDWR | O_TRUNC, 0666);
            printf("size:%d\n", size);
            write(outf, strBuf, size);
            close(outf);
        }
    }
    close(f);
    return 0;
}
*/