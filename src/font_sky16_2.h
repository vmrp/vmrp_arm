#ifndef FONT_SKY16_H
#define FONT_SKY16_H

#include "dsm.h"
#include "main.h"

extern uint16 *screenBuf;  //屏幕缓冲区地址


int xl_font_sky16_init();
int xl_font_sky16_close();


char *xl_font_sky16_getChar(uint16 id);
char *xl_font_sky16_drawChar(uint16 id, int x, int y, uint16 color);
void xl_font_sky16_charWidthHeight(uint16 id, int32 *width, int32 *height);

#endif
