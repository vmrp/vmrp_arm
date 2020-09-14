#ifndef FONT_SKY16_H
#define FONT_SKY16_H

#include "./mr/include/type.h"

#define EN_CHAR_H 16
#define EN_CHAR_W 8
#define CN_CHAR_H 16
#define CN_CHAR_W 16


int xl_font_sky16_init();
int xl_font_sky16_close();

char *xl_font_sky16_getChar(uint16 id);
char *xl_font_sky16_drawChar(uint16 id, int x, int y, uint16 color);
void xl_font_sky16_charWidthHeight(uint16 id, int32 *width, int32 *height);

#endif
