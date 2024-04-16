#include <stdio.h>

#define MEMORIA_SIZE 16384

typedef struct {
	char memoria[MEMORIA_SIZE];
	int registros[16];
	int tablaSegmentos[2];
	int errorFlag;
} TMV;

void registerMask(int secReg, int *corr, int *mask);

void iniciaMV(TMV *mv, int programSize);

int readHeader(int *programSize, char *filename,TMV *mv);

void cargaCodigo(TMV *mv, char *filename, int programSize);

char instruccionActual(TMV mv);

int instruccionValida(char codOp);

int segmentoCheck(TMV mv,int oprnd, int tipo);

void readOperand(TMV *mv, int tipo, int *operador);

int operandValue(TMV mv, int operand, int tipo);

int direccion(TMV mv, int ip);

void disassembler(TMV mv, int programSize);

int checkParam(int argc, char *argv[], char strCmp[]);

void pasoDis(TMV *mv, char instruccion);

void mostrarOp(int tipo, int valor);

void OperandDis(TMV *mv, int tipo, int *operador);

void fillExtraDis(int extraSpace);
