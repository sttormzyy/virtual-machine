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

void SYS(int A, int opA, int B, int opB, TMV *mv);

void JMP(int A, int opA, int B, int opB, TMV *mv);

void JZ(int A, int opA, int B, int opB, TMV *mv);

void JP(int A, int opA, int B, int opB, TMV *mv);

void JN(int A, int opA, int B, int opB, TMV *mv);

void JNZ(int A, int opA, int B, int opB, TMV *mv);

void JNP(int A, int opA, int B, int opB, TMV *mv);

void JNN(int A, int opA, int B, int opB, TMV *mv);

void LDL(int A, int opA, int B, int opB, TMV *mv);

void LDH(int A, int opA, int B, int opB, TMV *mv);

void NOT(int A, int opA, int B, int opB, TMV *mv);

void Input(int *x,int modo);

void Output(int x, int modo);
