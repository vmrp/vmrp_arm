        CODE32

        AREA ||.text||, CODE, READONLY

getR9 PROC
        MOV      r0,r9
        BX       lr
        ENDP

setR9 PROC
        MOV      r9,r0
        BX       lr
        ENDP

getR10 PROC
        MOV      r0,r10
        BX       lr
        ENDP

setR10 PROC
        MOV      r10,r0
        BX       lr
        ENDP



        EXPORT setR10
        EXPORT getR10
        EXPORT setR9
        EXPORT getR9

        END
