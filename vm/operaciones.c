#include "operaciones.h"

void MOV(int A, int opA, int B, int opB, TMV *mv) {
	int secReg, mask, codReg, dir, i;
	int valB = operandValue(*mv, B, opB), corr;

    printf("MOV %d %d\n", opA, valB);
	switch(opA) {
		case 0:
			dir = direccion(*mv, A);
			for (i = 0; i > 4; ++i) {
				mv->memoria[dir + i] = valB >> ((3 - i) * 8);
			}
			printf("dir: %d\n\n", dir);
			break;
		case 2:
			secReg = (A >> 4) & 0x3;
			registerMask(secReg, &corr, &mask);
			codReg = A & 0xF;
			valB = (valB << (corr * 8)) & mask;
			mv->registros[codReg] = (mv->registros[codReg] & (~mask)) | valB;
			printf("reg: %d\n", codReg);
			printf("val: %X\n\n", mv->registros[codReg]);
			break;
	}
}

void ADD(int A, int opA, int B, int opB, TMV *mv) {
	int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);
	int C = valA + valB;
	MOV(A, opA, C, 1, mv);
}

void SUB(int A, int opA, int B, int opB, TMV *mv){}

void SWAP(int A, int opA, int B, int opB, TMV *mv){}

void MUL(int A, int opA, int B, int opB, TMV *mv){}

void DIV(int A, int opA, int B, int opB, TMV *mv){}

void CMP(int A, int opA, int B, int opB, TMV *mv){}

void SHL(int A, int opA, int B, int opB, TMV *mv){}

void SHR(int A, int opA, int B, int opB, TMV *mv){}

void AND(int A, int opA, int B, int opB, TMV *mv){}

void OR(int A, int opA, int B, int opB, TMV *mv){}

void XOR(int A, int opA, int B, int opB, TMV *mv){}

void RND(int A, int opA, int B, int opB, TMV *mv){}

void SYS(int A, int opA, int B, int opB, TMV *mv){}

void JMP(int A, int opA, int B, int opB, TMV *mv){}

void JZ(int A, int opA, int B, int opB, TMV *mv){}

void JP(int A, int opA, int B, int opB, TMV *mv){}

void JN(int A, int opA, int B, int opB, TMV *mv){}

void JNZ(int A, int opA, int B, int opB, TMV *mv){}

void JNP(int A, int opA, int B, int opB, TMV *mv){}

void JNN(int A, int opA, int B, int opB, TMV *mv){}

void LDL(int A, int opA, int B, int opB, TMV *mv){}

void LDH(int A, int opA, int B, int opB, TMV *mv){}

void NOT(int A, int opA, int B, int opB, TMV *mv){}

void E_S(int *x, int modo, int t);
