#include <stdio.h>

#define MEMORIA_SIZE 16384

typedef struct {
	char memoria[MEMORIA_SIZE];
	int registros[16];
	int tablaSegmentos[2];
} TMV;

void registerMask(int secReg, int *corr, int *mask);
void iniciaMV(TMV *mv, int programSize);
int leeHeader(int *programSize, char *filename);
void cargaCodigo(TMV *mv, char *filename, int programSize);
char instruccionActual(TMV mv);
void readOperand(TMV *mv, int tipo, int *operador);
int operandValue(TMV mv, int operand, int tipo);
int direccion(TMV mv, int IP);
