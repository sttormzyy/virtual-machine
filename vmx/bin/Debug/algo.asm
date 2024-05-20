TEXTO1 EQU "HOLA MUNDO"
TEXTO2 EQU "AYUDA, ME ESTAN MATANDO"
TEXTO3 EQU "LMAO"

_start: MOV EDX, DS
        ADD EDX, 0
        SYS %3
        SYS %F
        MOV EBX,4
        MOV EFX,5
        SYS %F
        MUL EFX,EBX
        MOV EDX,KS
        ADD EDX,TEXTO3
        SYS %4
        SYS %F
        MOV EDX,DS
        ADD EDX,20
        MOV [EDX],EFX
        MOV EAX,%F
        MOV CX,%401
        SYS 2
