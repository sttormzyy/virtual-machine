TEXTO1 EQU "HOLA MUNDO"
TEXTO2 EQU "AYUDA, ME ESTAN MATANDO"
TEXTO3 EQU "LMAO"

_start: MOV EDX, DS
        ADD EDX, 0
        mov b[DS], 'a;
        mov b[DS+1], 'b;
        mov b[DS+2], 'c;
        mov b[DS+3], 10;
        mov b[DS+4], 11;
        mov b[DS+4], 12;
        SYS %4
        MOV EDX, KS
        ADD EDX, TEXTO2
        mov b[EDX+2], 10;
        SYS %4
        MOV EDX, KS
        ADD EDX, TEXTO3
        SYS %4