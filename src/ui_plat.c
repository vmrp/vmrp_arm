/*
 实现平台UI
 风的影子
*/

#include "./include/encode.h"
#include "./include/fixR9.h"
#include "./include/md5.h"
#include "./include/mem.h"
#include "./include/mr.h"
#include "./include/mr_auxlib.h"
#include "./include/mr_base64.h"
#include "./include/mr_graphics.h"
#include "./include/mr_gzip.h"
#include "./include/mr_helper.h"
#include "./include/mr_lib.h"
#include "./include/mr_socket_target.h"
#include "./include/mr_store.h"
#include "./include/mr_tcp_target.h"
#include "./include/mrporting.h"
#include "./include/other.h"
#include "./include/dsm.h"

void mrc_clearScreen(int32 r, int32 g, int32 b);
int32 mrc_drawText(char *pcText, int16 x, int16 y, uint8 r, uint8 g, uint8 b, int is_unicode, uint16 font);
void mrc_refreshScreen(int16 x, int16 y, uint16 w, uint16 h);
extern int32 mr_state;
extern void ui_draw();
extern int32 ui_event(int32 type,int32 p1,int32 p2);
// int SCREEN_WIDTH,SCREEN_HEIGHT;


typedef struct UI_DIALOG{
    char *title;
    char *text;
    int32 title_len;
    int32 text_len;
    int32 type;
} UI_DIALOG;

typedef struct UI_EDIT{
    char *title;
    char text[256];
    int32 title_len;
    int32 type;
    int32 max_size;
} UI_EDIT;

typedef struct UI_BUTTON{
    char *text;
    int32 x;
    int32 y;
    int32 w;
    int32 h;
    struct UI_BUTTON *next;
} UI_BUTTON;

UI_EDIT *ui_edit = NULL;
UI_DIALOG *ui_dlg = NULL;
UI_BUTTON *ui_button = NULL;

//创建对话框
int32 mr_dialogCreate(const char *title, const char *text, int32 type){
    UI_DIALOG *dlg = MR_MALLOC(sizeof(UI_DIALOG));
    memset2(dlg,0,sizeof(UI_DIALOG));
    if(title!=NULL){
        dlg->title = MR_MALLOC(wstrlen((char*)title)+2);
        dlg->title_len = wstrlen((char*)title)+2;
        strncpy2(dlg->title, title, wstrlen((char*)title)+2);
    }
    if(text!=NULL){
        dlg->text = MR_MALLOC(wstrlen((char*)text)+2);
        dlg->text_len = wstrlen((char*)text)+2;
        strncpy2(dlg->text, text, wstrlen((char*)text)+2);
    }
    dlg->type = type;
    if(mr_state == MR_STATE_RUN)
    mr_state = MR_STATE_UI;
    return (int32)dlg;
}

//释放对话框
int32 mr_dialogRelease(int32 id) {
    if(id){
        UI_DIALOG *dlg = (UI_DIALOG*) id;
        if(dlg->title){
            MR_FREE(dlg->title, dlg->title_len);
        }
        if(dlg->text){
            MR_FREE(dlg->text, dlg->text_len);
        }
        MR_FREE(dlg,sizeof(UI_DIALOG));
    }
    return -1;
}

int32 mr_dialogRefresh(int32 dialog, const char *title, const char *text, int32 type){
    mrc_refreshScreen(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
}

UI_BUTTON* button_create(int32 x,int32 y,int32 w,int32 h){
    UI_BUTTON *ui = MR_MALLOC(sizeof(UI_BUTTON));
    memset2(ui, 0, sizeof(UI_BUTTON));
    ui-> x = x;
    ui-> y = y;
    ui-> w = w;
    ui-> h = h;
    return ui;
}

int32 mr_editCreate(const char *title, const char *text, int32 type, int32 max_size) {
    UI_EDIT *edit = MR_MALLOC(sizeof(UI_EDIT));
    LOGI("mr_editCreate %d\n", type);
    memset2(edit,0,sizeof(UI_EDIT));
    if(title!=NULL){
        edit->title = MR_MALLOC(wstrlen((char*)title)+2);
        strncpy2(edit->title, title, wstrlen((char*)title)+2);
    }
    if(text!=NULL){
        strncpy2(edit->text, text, wstrlen((char*)text)+2);
    }
    edit->type = type;
    edit->max_size = max_size;
    if(mr_state == MR_STATE_RUN)
    mr_state = MR_STATE_UI;
    ui_edit = edit;

    LOGI("mr_editCreate %s\n", "ui_draw");
    ui_draw();
    // ui_button = button_create(0,y, w,h);
    // ui_button->next = 

    return (int32)edit;
}

int32 mr_editRelease(int32 id) {
    LOGI("mr_editRelease %d\n", id);
    if(id){
        UI_EDIT *edit = (UI_EDIT*)id;
        MR_FREE(edit,sizeof(UI_DIALOG));
        return 0;
    }
    return -1;
}

const char *mr_editGetText(int32 id) {
    if(id){
        UI_EDIT *edit = (UI_EDIT*)id;
        return edit->text;
    }
    return NULL;
}

void ui_draw(){
    int32 w = SCREEN_WIDTH/3;
    int32 h = SCREEN_HEIGHT/8;
    int32 y = SCREEN_HEIGHT-h*4;
    LOGI("ui_draw %s\n","");
    if(mr_state == MR_STATE_UI){
        mrc_clearScreen(255,255,255);
        //绘制编辑框
        mrc_drawText(ui_edit->text, 10, 10, 20, 20, 20, 1, 1);

        //绘制数字
        mrc_drawText("\x63\x9\x4e\xb\x95\x2e\x76\xd8\x8f\xdb\x88\x4c\x8f\x93\x51\x65\x0\x0", 10,y, 50,50,50, 1, 1);
        mrc_drawText("\x63\x9\x5d\xe6\x65\xb9\x54\x11\x95\x2e\x90\x0\x68\x3c\x0\x0", 10,y+26, 50,50,50, 1, 1);
        // mrc_drawText("3", w*2,y, 20,20,20, 0, 1);

        // mrc_drawText("4", 0,y+h, 20,20,20, 0, 1);
        // mrc_drawText("5", w,y+h, 20,20,20, 0, 1);
        // mrc_drawText("6", w*2,y+h, 20,20,20, 0, 1);

        // mrc_drawText("7", 0,y+h*2, 20,20,20, 0, 1);
        // mrc_drawText("8", w,y+h*2, 20,20,20, 0, 1);
        // mrc_drawText("9", w*2,y+h*2, 20,20,20, 0, 1);

        // mrc_drawText("0", 0,y+h*3, 20,20,20, 0, 1);
        mrc_refreshScreen(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
LOGI("ui_draw ok %s\n","");
        return ;
    }
}

int32 ui_event(int32 type,int32 p1,int32 p2){
    int32 edit_len;
    LOGI("ui_event %d %d %d\n",type, p1, p2);
    // if(mr_state == MR_STATE_UI){
        //绘制编辑框
        
        edit_len = wstrlen(ui_edit->text);
        

        if(type == MR_KEY_RELEASE){
            switch(p1){
                case MR_KEY_0:
                ui_edit->text[edit_len+1] = '0';
                edit_len += 2;
                break;
                case MR_KEY_1:
                ui_edit->text[edit_len+1] = '1';
                edit_len += 2;
                break;
                case MR_KEY_2:
                ui_edit->text[edit_len+1] = '2';
                edit_len += 2;
                break;
                case MR_KEY_3:
                ui_edit->text[edit_len+1] = '3';
                edit_len += 2;
                break;
                case MR_KEY_4:
                ui_edit->text[edit_len+1] = '4';
                edit_len += 2;
                break;
                case MR_KEY_5:
                ui_edit->text[edit_len+1] = '5';
                edit_len += 2;
                break;
                case MR_KEY_6:
                ui_edit->text[edit_len+1] = '6';
                edit_len += 2;
                break;
                case MR_KEY_7:
                ui_edit->text[edit_len+1] = '7';
                edit_len += 2;
                break;
                case MR_KEY_8:
                ui_edit->text[edit_len+1] = '8';
                edit_len += 2;
                break;
                case MR_KEY_9:
                ui_edit->text[edit_len+1] = '9';
                edit_len += 2;
                break;
                case MR_KEY_LETTER_A:
                case MR_KEY_LETTER_B:
                case MR_KEY_LETTER_C:
                case MR_KEY_LETTER_D:
                case MR_KEY_LETTER_E:
                case MR_KEY_LETTER_F:
                case MR_KEY_LETTER_G:
                case MR_KEY_LETTER_H:
                case MR_KEY_LETTER_I:
                case MR_KEY_LETTER_J:
                case MR_KEY_LETTER_K:
                case MR_KEY_LETTER_L:
                case MR_KEY_LETTER_M:
                case MR_KEY_LETTER_N:
                case MR_KEY_LETTER_O:
                case MR_KEY_LETTER_P:
                case MR_KEY_LETTER_Q:
                case MR_KEY_LETTER_R:
                case MR_KEY_LETTER_S:
                case MR_KEY_LETTER_T:
                case MR_KEY_LETTER_U:
                case MR_KEY_LETTER_V:
                case MR_KEY_LETTER_W:
                case MR_KEY_LETTER_X:
                case MR_KEY_LETTER_Y:
                case MR_KEY_LETTER_Z:
                case MR_KEY_LETTER_UNDERSCORE:
                case MR_KEY_LETTER_BACKQUOTE:
                case MR_KEY_LETTER_CARET:
                case MR_KEY_LETTER_QUESTION:
                case MR_KEY_LETTER_AT:
                case MR_KEY_LETTER_SPACE:
                    if(ui_edit->type != MR_EDIT_NUMERIC){
                        ui_edit->text[edit_len+1] = p1;
                        edit_len += 2;
                    }
                break;
                case MR_KEY_LEFT:
                ui_edit->text[edit_len-1] = 0;
                ui_edit->text[edit_len-2] = 0;
                edit_len -= 2;
                break;
                case MR_KEY_SOFTLEFT:
                mr_state = MR_STATE_RUN;
                mr_event(MR_DIALOG_EVENT, 0, 0);
                break;
                case MR_KEY_SOFTRIGHT:
                mr_state = MR_STATE_RUN;
                mr_event(MR_DIALOG_EVENT, 1, 0);
            }
            ui_draw();
        }
        

        return 0;
}