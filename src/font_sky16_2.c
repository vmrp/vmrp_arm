#include "font_sky16_2.h"

#include "dsm.h"
#include "main.h"

/*
 sky字体绘制
 风的影子
 */

typedef struct {
    char *bitbuf;
    char *filename;  //字体文件名
    long f;          //文件指针
    long font_size;  //字号
    long ansi_size;  //ansi字符宽度
    int width;
    int height;
} FONT;

static char font_sky16_bitbuf[32];

int font_sky16_f;

int xl_font_sky16_init() {  //字体初始化，打开字体文件
    font_sky16_f = mr_open("system/gb16_mrpoid.uc2", 0);
    if (font_sky16_f <= 0) {
        LOGW("字体加载失败");
        return -1;
    }
    LOGI("字体加载成功 fd=%d", font_sky16_f);
    return 0;
}

int xl_font_sky16_close() {  //关闭字体
    LOGW("关闭字体");
    return mr_close(font_sky16_f);
}

static void dpoint(int x, int y, int color) {
    if (x < 0 || x >= SCRW || y < 0 || y >= SCRH)
        return;
    *(screenBuf + y * SCRW + x) = color;
}

//获取字体第n个点信息
static int getfontpix(char *buf, int n) {
    buf += n / 8;  //计算在第几个字节，从0开始
    //计算在第几位n%8
    return (128 >> (n % 8)) & *buf;
}

//获得字符的位图
char *xl_font_sky16_getChar(uint16 id) {
    mr_seek(font_sky16_f, id * 32, 0);
    mr_read(font_sky16_f, font_sky16_bitbuf, 32);
    return font_sky16_bitbuf;
}

//画一个字
char *xl_font_sky16_drawChar(uint16 id, int x, int y, uint16 color) {
    mr_seek(font_sky16_f, id * 32, 0);
    mr_read(font_sky16_f, font_sky16_bitbuf, 32);
    int y2 = y + 16;
    int n = 0, count;

    int ix = x;
    int iy;
    for (iy = y; iy < y2; iy++) {
        ix = x;
        for (count = 0; count < 16; count++) {
            if (getfontpix(font_sky16_bitbuf, n))
                dpoint(ix, iy, color);
            n++, ix++;
        }
    }
    return font_sky16_bitbuf;
}

//获取一个文字的宽高
void xl_font_sky16_charWidthHeight(uint16 id, int32 *width, int32 *height) {
    if (id < 128) {
        if (width) *width = 8;
        if (height) *height = 16;
        return;
    }
    if (width) *width = 16;
    if (height) *height = 16;
}

//获取单行文字的宽高
void xl_font_sky16_textWidthHeight(char *text, int32 *width, int32 *height) {
    int textIdx = 0;
    int id;
    int fontw = 0, fonth = 0;
    while (text[textIdx] != 0) {
        id = (text[textIdx] << 8) + (text[textIdx + 1]);
        xl_font_sky16_charWidthHeight(id, &fontw, &fonth);
        (*width) += fontw;
        (*height) += fonth;
        textIdx += 2;
    }
}
