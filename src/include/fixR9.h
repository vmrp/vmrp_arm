#ifndef __Fix_R9__
#define __Fix_R9__

#include "mr.h"
#include "mr_graphics.h"
#include "mr_gzip.h"
#include "mythroad.h"
#include "type.h"

#ifdef __GNUC__
#define fixR9_saveMythroad()
extern void *getR9(void);
extern void setR9(void *value);
extern void *getR10(void);
extern void setR10(void *value);

#define asm_mr_malloc mr_malloc
#define asm_mr_free mr_free
#define asm_mr_getDatetime mr_getDatetime
#define asm_mr_sleep mr_sleep
#define asm_DrawRect DrawRect
#define asm_mr_drawBitmap mr_drawBitmap
#define asm_DrawText _DrawText
#define asm_mr_getScreenInfo mr_getScreenInfo
#define asm_mr_smsSetBytes _mr_smsSetBytes
#define asm_mr_smsAddNum _mr_smsAddNum
#define asm_mr_newSIMInd _mr_newSIMInd
#define asm_mr_isMr _mr_isMr
#define asm_mr_realloc mr_realloc
#define asm_mr_rand mr_rand
#define asm_mr_stop_ex mr_stop_ex
#define asm_mr_printf mr_printf
#define asm_mr_mem_get mr_mem_get
#define asm_mr_mem_free mr_mem_free
#define asm_mr_getCharBitmap mr_getCharBitmap
#define asm_mr_timerStart mr_timerStart
#define asm_mr_timerStop mr_timerStop
#define asm_mr_getTime mr_getTime
#define asm_mr_getUserInfo mr_getUserInfo
#define asm_mr_plat mr_plat
#define asm_mr_platEx mr_platEx
#define asm_mr_open mr_open
#define asm_mr_close mr_close
#define asm_mr_read mr_read
#define asm_mr_write mr_write
#define asm_mr_seek mr_seek
#define asm_mr_info mr_info
#define asm_mr_remove mr_remove
#define asm_mr_rename mr_rename
#define asm_mr_mkDir mr_mkDir
#define asm_mr_rmDir mr_rmDir
#define asm_mr_findGetNext mr_findGetNext
#define asm_mr_findStop mr_findStop
#define asm_mr_findStart mr_findStart
#define asm_mr_getLen mr_getLen
#define asm_mr_exit mr_exit
#define asm_mr_sendSms mr_sendSms
#define asm_mr_call mr_call
#define asm_mr_connectWAP mr_connectWAP
#define asm_mr_initNetwork mr_initNetwork
#define asm_mr_closeNetwork mr_closeNetwork
#define asm_mr_socket mr_socket
#define asm_mr_connect mr_connect
#define asm_mr_closeSocket mr_closeSocket
#define asm_mr_recv mr_recv
#define asm_mr_recvfrom mr_recvfrom
#define asm_mr_send mr_send
#define asm_mr_sendto mr_sendto
#define asm_mr_load_sms_cfg _mr_load_sms_cfg
#define asm_mr_save_sms_cfg _mr_save_sms_cfg
#define asm_DispUpEx _DispUpEx
#define asm_DrawPoint _DrawPoint
#define asm_DrawBitmap _DrawBitmap
#define asm_DrawBitmapEx _DrawBitmapEx
#define asm_BitmapCheck _BitmapCheck
#define asm_mr_readFile _mr_readFile
#define asm_mr_registerAPP mr_registerAPP
#define asm_DrawTextEx _DrawTextEx
#define asm_mr_EffSetCon _mr_EffSetCon
#define asm_mr_TestCom _mr_TestCom
#define asm_mr_TestCom1 _mr_TestCom1
#define asm_c2u c2u
#define asm_mr_updcrc mr_updcrc
#define asm_mr_unzip mr_unzip
#define asm_mr_transbitmapDraw mr_transbitmapDraw
#define asm_mr_drawRegion mr_drawRegion
#define asm_mr_platDrawChar mr_platDrawChar
#else

void fixR9_saveMythroad(void);

// 以下函数全部在汇编代码实现

extern void *getR9(void);
extern void setR9(void *value);
extern void *getR10(void);
extern void setR10(void *value);
extern void setR9R10(void *r9, void *r10);

extern void *asm_mr_malloc(uint32 len);
extern void asm_mr_free(void *p, uint32 len);
extern int32 asm_mr_getDatetime(mr_datetime *datetime);
extern int32 asm_mr_sleep(uint32 ms);
extern void asm_DrawRect(int16 x, int16 y, int16 w, int16 h, uint8 r, uint8 g, uint8 b);
extern void asm_mr_drawBitmap(uint16 *bmp, int16 x, int16 y, uint16 w, uint16 h);
extern int32 asm_DrawText(char *pcText, int16 x, int16 y, uint8 r, uint8 g, uint8 b, int is_unicode, uint16 font);
extern int32 asm_mr_getScreenInfo(mr_screeninfo *s);
extern int32 asm_mr_smsSetBytes(int32 pos, char *p, int32 len);
extern int32 asm_mr_smsAddNum(int32 index, char *pNum);
extern int32 asm_mr_newSIMInd(int16 type, uint8 *old_IMSI);
extern int asm_mr_isMr(char *input);
extern void *asm_mr_realloc(void *p, uint32 oldlen, uint32 len);
extern int32 asm_mr_rand(void);
extern int32 asm_mr_stop_ex(int16 freemem);
extern void asm_mr_printf(const char *format, ...);
extern int32 asm_mr_mem_get(char **mem_base, uint32 *mem_len);
extern int32 asm_mr_mem_free(char *mem, uint32 mem_len);
extern const char *asm_mr_getCharBitmap(uint16 ch, uint16 fontSize, int *width, int *height);
extern int32 asm_mr_timerStart(uint16 t);
extern int32 asm_mr_timerStop(void);
extern uint32 asm_mr_getTime(void);
extern int32 asm_mr_getUserInfo(mr_userinfo *info);
extern int32 asm_mr_plat(int32 code, int32 param);
extern int32 asm_mr_platEx(int32 code, uint8 *input, int32 input_len, uint8 **output, int32 *output_len, MR_PLAT_EX_CB *cb);
extern int32 asm_mr_open(const char *filename, uint32 mode);
extern int32 asm_mr_close(int32 f);
extern int32 asm_mr_read(int32 f, void *p, uint32 l);
extern int32 asm_mr_write(int32 f, void *p, uint32 l);
extern int32 asm_mr_seek(int32 f, int32 pos, int method);
extern int32 asm_mr_info(const char *filename);
extern int32 asm_mr_remove(const char *filename);
extern int32 asm_mr_rename(const char *oldname, const char *newname);
extern int32 asm_mr_mkDir(const char *name);
extern int32 asm_mr_rmDir(const char *name);
extern int32 asm_mr_findGetNext(int32 search_handle, char *buffer, uint32 len);
extern int32 asm_mr_findStop(int32 search_handle);
extern int32 asm_mr_findStart(const char *name, char *buffer, uint32 len);
extern int32 asm_mr_getLen(const char *filename);
extern int32 asm_mr_exit(void);
extern int32 asm_mr_sendSms(char *pNumber, char *pContent, int32 encode);
extern void asm_mr_call(char *number);
extern void asm_mr_connectWAP(char *wap);
extern int32 asm_mr_initNetwork(MR_INIT_NETWORK_CB cb, const char *mode);
extern int32 asm_mr_closeNetwork(void);
extern int32 asm_mr_socket(int32 type, int32 protocol);
extern int32 asm_mr_connect(int32 s, int32 ip, uint16 port, int32 type);
extern int32 asm_mr_closeSocket(int32 s);
extern int32 asm_mr_recv(int32 s, char *buf, int len);
extern int32 asm_mr_recvfrom(int32 s, char *buf, int len, int32 *ip, uint16 *port);
extern int32 asm_mr_send(int32 s, const char *buf, int len);
extern int32 asm_mr_sendto(int32 s, const char *buf, int len, int32 ip, uint16 port);
extern int32 asm_mr_load_sms_cfg(void);
extern int32 asm_mr_save_sms_cfg(int32 f);
extern int32 asm_DispUpEx(int16 x, int16 y, uint16 w, uint16 h);
extern void asm_DrawPoint(int16 x, int16 y, uint16 nativecolor);
extern void asm_DrawBitmap(uint16 *p, int16 x, int16 y, uint16 w, uint16 h, uint16 rop, uint16 transcoler, int16 sx, int16 sy, int16 mw);
extern void asm_DrawBitmapEx(mr_bitmapDrawSt *srcbmp, mr_bitmapDrawSt *dstbmp, uint16 w, uint16 h, mr_transMatrixSt *pTrans, uint16 transcoler);
extern int asm_BitmapCheck(uint16 *p, int16 x, int16 y, uint16 w, uint16 h, uint16 transcoler, uint16 color_check);
extern void *asm_mr_readFile(const char *filename, int *filelen, int lookfor);
extern int32 asm_mr_registerAPP(uint8 *p, int32 len, int32 index);
extern int32 asm_DrawTextEx(char *pcText, int16 x, int16 y, mr_screenRectSt rect, mr_colourSt colorst, int flag, uint16 font);
extern int asm_mr_EffSetCon(int16 x, int16 y, int16 w, int16 h, int16 perr, int16 perg, int16 perb);
extern int asm_mr_TestCom(mrp_State *L, int input0, int input1);
extern int asm_mr_TestCom1(mrp_State *L, int input0, char *input1, int32 len);
extern uint16 *asm_c2u(const char *cp, int *err, int *size);
extern uint32 asm_mr_updcrc(uint8 *s, unsigned n);
extern int asm_mr_unzip(void);
extern int32 asm_mr_transbitmapDraw(mr_transBitmap *hTransBmp, uint16 *dstBuf, int32 dest_max_w, int32 dest_max_h, int32 sx, int32 sy, int32 width, int32 height, int32 dx, int32 dy);
extern void asm_mr_drawRegion(mr_jgraphics_context_t *gContext, mr_jImageSt *src, int sx, int sy, int w, int h, int transform, int x, int y, int anchor);
extern void asm_mr_platDrawChar(uint16 ch, int32 x, int32 y, uint32 color);
#endif

#endif
