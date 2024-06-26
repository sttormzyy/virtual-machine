#include "mv.h"

void MOV(int A, int opA, int B, int opB, TMV *mv);

void ADD(int A, int opA, int B, int opB, TMV *mv);

void SUB(int A, int opA, int B, int opB, TMV *mv);

void SWAP(int A, int opA, int B, int opB, TMV *mv);

void MUL(int A, int opA, int B, int opB, TMV *mv);

void DIV(int A, int opA, int B, int opB, TMV *mv);

void CMP(int A, int opA, int B, int opB, TMV *mv);

void SHL(int A, int opA, int B, int opB, TMV *mv);

void SHR(int A, int opA, int B, int opB, TMV *mv);

void AND(int A, int opA, int B, int opB, TMV *mv);

void OR(int A, int opA, int B, int opB, TMV *mv);

void XOR(int A, int opA, int B, int opB, TMV *mv);

void RND(int A, int opA, int B, int opB, TMV *mv);

void SYS(int A, int opA, TMV *mv);

void jump(int salto, int codOp, TMV *mv, void (*operaciones[])());

void JMP(int salto, TMV *mv);

void JZ(int salto, TMV *mv);

void JP(int salto, TMV *mv);

void JN(int salto, TMV *mv);

void JNZ(int salto, TMV *mv);

void JNP(int salto, TMV *mv);

void JNN(int salto, TMV *mv);

void LDL(int A, int opA, TMV *mv);

void LDH(int A, int opA, TMV *mv);

void NOT(int A, int opA, TMV *mv);

void PUSH(int opB, int B, TMV *mv);

void POP(int opA, int A, TMV *mv);

void CALL(int salto, int codOp, TMV *mv);

void RET(int A, int opB, TMV *mv);

void SYS1(TMV *mv);

void SYS2(TMV *mv);

void SYS3(TMV *mv);

void SYS4(TMV *mv);

void SYS7(TMV *mv);

void SYSF(TMV *mv);

void input(int *x,int modo);

void output(int x, int modo, int tam);

void pasoDebug(TMV *mv, char* filename);


