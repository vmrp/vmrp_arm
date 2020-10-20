// node getR9R10.js

const obj = {
    'asm_mr_malloc': 'mr_malloc',
    'asm_mr_free': 'mr_free',
    'asm_mr_getDatetime': 'mr_getDatetime',
    'asm_mr_sleep': 'mr_sleep',
    'asm_DrawRect': 'DrawRect',
    'asm_mr_drawBitmap': 'mr_drawBitmap',
    'asm_DrawText': '_DrawText',
    'asm_mr_getScreenInfo': 'mr_getScreenInfo',
    'asm_mr_smsSetBytes': '_mr_smsSetBytes',
    'asm_mr_smsAddNum': '_mr_smsAddNum',
    'asm_mr_newSIMInd': '_mr_newSIMInd',
    'asm_mr_isMr': '_mr_isMr',
    'asm_mr_realloc': 'mr_realloc',
    'asm_mr_rand': 'mr_rand',
    'asm_mr_stop_ex': 'mr_stop_ex',
    'asm_mr_printf': 'mr_printf',
    'asm_mr_mem_get': 'mr_mem_get',
    'asm_mr_mem_free': 'mr_mem_free',
    'asm_mr_getCharBitmap': 'mr_getCharBitmap',
    'asm_mr_timerStart': 'mr_timerStart',
    'asm_mr_timerStop': 'mr_timerStop',
    'asm_mr_getTime': 'mr_getTime',
    'asm_mr_getUserInfo': 'mr_getUserInfo',
    'asm_mr_plat': 'mr_plat',
    'asm_mr_platEx': 'mr_platEx',
    'asm_mr_open': 'mr_open',
    'asm_mr_close': 'mr_close',
    'asm_mr_read': 'mr_read',
    'asm_mr_write': 'mr_write',
    'asm_mr_seek': 'mr_seek',
    'asm_mr_info': 'mr_info',
    'asm_mr_remove': 'mr_remove',
    'asm_mr_rename': 'mr_rename',
    'asm_mr_mkDir': 'mr_mkDir',
    'asm_mr_rmDir': 'mr_rmDir',
    'asm_mr_findGetNext': 'mr_findGetNext',
    'asm_mr_findStop': 'mr_findStop',
    'asm_mr_findStart': 'mr_findStart',
    'asm_mr_getLen': 'mr_getLen',
    'asm_mr_exit': 'mr_exit',
    'asm_mr_sendSms': 'mr_sendSms',
    'asm_mr_call': 'mr_call',
    'asm_mr_connectWAP': 'mr_connectWAP',
    'asm_mr_initNetwork': 'mr_initNetwork',
    'asm_mr_closeNetwork': 'mr_closeNetwork',
    'asm_mr_socket': 'mr_socket',
    'asm_mr_connect': 'mr_connect',
    'asm_mr_closeSocket': 'mr_closeSocket',
    'asm_mr_recv': 'mr_recv',
    'asm_mr_recvfrom': 'mr_recvfrom',
    'asm_mr_send': 'mr_send',
    'asm_mr_sendto': 'mr_sendto',
    'asm_mr_load_sms_cfg': '_mr_load_sms_cfg',
    'asm_mr_save_sms_cfg': '_mr_save_sms_cfg',
    'asm_DispUpEx': '_DispUpEx',
    'asm_DrawPoint': '_DrawPoint',
    'asm_DrawBitmap': '_DrawBitmap',
    'asm_DrawBitmapEx': '_DrawBitmapEx',
    'asm_BitmapCheck': '_BitmapCheck',
    'asm_mr_readFile': '_mr_readFile',
    'asm_mr_registerAPP': 'mr_registerAPP',
    'asm_DrawTextEx': '_DrawTextEx',
    'asm_mr_EffSetCon': '_mr_EffSetCon',
    'asm_mr_TestCom': '_mr_TestCom',
    'asm_mr_TestCom1': '_mr_TestCom1',
    'asm_c2u': 'c2u',
    'asm_mr_updcrc': 'mr_updcrc',
    'asm_mr_unzip': 'mr_unzip',
    'asm_mr_transbitmapDraw': 'mr_transbitmapDraw',
    'asm_mr_drawRegion': 'mr_drawRegion',
    'asm_mr_platDrawChar': 'mr_platDrawChar',
}

const asm = `
        CODE32
        AREA ||.text||, CODE, READONLY
        IMPORT       fixR9_begin
        IMPORT       fixR9_end
        IMPORT       fixR9_saveLR
        IMPORT       fixR9_getLR

getR9 PROC
        MOV      r0,r9
        BX       lr
        ENDP
        EXPORT getR9

setR9 PROC
        MOV      r9,r0
        BX       lr
        ENDP
        EXPORT setR9

getR10 PROC
        MOV      r0,r10
        BX       lr
        ENDP
        EXPORT getR10

setR10 PROC
        MOV      r10,r0
        BX       lr
        ENDP
        EXPORT setR10

setR9R10 PROC
        MOV      r9,r0
        MOV      r10,r1
        BX       lr
        ENDP
        EXPORT setR9R10
    
{{replace}}

        END
`;

const tpl = `
        IMPORT   {{targetFuncName}}
{{asmFuncName}} PROC
        stmfd    sp,{ r0-r8, r11, r12, sp, lr } ; 因为不确定fixR9_xxx的c函数编译后会使用哪些寄存器，所以干脆全部保存
        sub      sp,sp,#52      ; r0-r8, r11, r12, sp, lr 一共13个寄存器 13*4=52
        bl       fixR9_begin
        ldmfd    sp,{ r0-r8, r11, r12, sp, lr }
        sub      sp,sp,#52      ; 此时已经回到mythroad空间，需要在全局变量保存lr的值，因为栈内容没变，所以不用重复保存，直接修改sp
        mov      r0,lr
        bl       fixR9_saveLR
        ldmfd    sp,{ r0-r8, r11, r12, sp, lr } ; 现在完全恢复调用参数
        bl       {{targetFuncName}}      ; 调用目标函数
        stmfd    sp,{ r0-r8, r11, r12, sp } ; 注意这里没有保存lr，因为lr的值已经在调用目标函数后破坏
        sub      sp,sp,#48      ; 12个寄存器
        bl       fixR9_getLR
        mov      lr,r0
        ldmfd    sp,{ r0-r8, r11, r12, sp }
        stmfd    sp,{ r0-r8, r11, r12, sp, lr } ; 因为不确定fixR9_xxx的c函数编译后会使用哪些寄存器，所以干脆全部保存
        sub      sp,sp,#52      ; r0-r8, r11, r12, sp, lr 一共13个寄存器 13*4=52
        bl       fixR9_end
        ldmfd    sp,{ r0-r8, r11, r12, sp, lr }
        bx       lr
        ENDP
        EXPORT {{asmFuncName}}
`;

const fs = require('fs');

function getAsmStr() {
    const arr = [];
    Object.keys(obj).forEach(function (key) {
        arr.push(tpl.replace(/\{\{asmFuncName\}\}/g, key).replace(/\{\{targetFuncName\}\}/g, obj[key]));
    });
    return arr.join('');
}

fs.writeFileSync('./r9r10.s', asm.replace('{{replace}}', getAsmStr()));
console.log('done.');

