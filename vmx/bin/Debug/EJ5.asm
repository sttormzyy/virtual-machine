string1 equ "Escriba mensaje cifrado:."
string2 equ "Escriba clave: "
SYS 1
CMP b[EDX], -1
JZ  algo
XOR b[EDX], b[EBX]
ADD EBX, 1
ADD EDX, 1
CMP b[EBX], 0
JNZ 0
MOV EBX, DS
JMP 0
algo: MOV b[EDX], 0
MOV EDX, ES
MOV ECX, -1
SYS 4
STOP
_start: SYS %F
MOV EDX, KS
ADD EDX,  26
MOV ECX, -1
SYS 4
MOV EDX, DS
MOV ECX, -1
SYS 3
MOV EBX, DS
MOV EDX, KS
ADD EDX, 0
MOV ECX, -1
SYS 4
MOV EDX, ES
MOV ECX, 257
MOV EAX, 8
JMP 0