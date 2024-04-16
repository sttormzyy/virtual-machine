// strings constantes utilizados en el disassembler
#define CS  0
#define DS  1
#define ES  2
#define SS  3
#define KS  4
#define IP  5
#define SP  6
#define BP  7
#define CC  8
#define AC  9
#define EAX  10
#define EBX  11
#define ECX  12
#define EDX  13
#define EEX  14
#define EFX  15

const char* fnNombres[] = {"MOV", "ADD", "SUB", "SWAP", "MUL", "DIV", "CMP", "SHL", "SHR", "AND", "OR", "XOR",
                           "RND", "NULL", "NULL", "NULL", "SYS %", "JMP", "JZ", "JP", "JN", "JNZ", "JNP", "JNN", "LDL", "LDH", "NOT",
                           "NULL","NULL","NULL","STOP","STOP"};

const char* regNombres[] = {"CS","DS","ES","SS","KS","IP","SP","BP","CC","AC",
                          "EAX","AL","AH","AX","EBX","BL","BH","BX","ECX","CL","CH","CX","EDX","DL","DH","DX",
                          "EEX","EL","EH","EX","EFX","FL","FH","FX"};
