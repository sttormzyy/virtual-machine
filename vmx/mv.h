#include "constantes.h"
#include <stdio.h>
#define MEMORIA_SIZE 16384

typedef struct {
	char* memoria;
	int memorySize;
	int registros[16];
	int tablaSegmentos[8];
	int errorFlag;
	int modo;
} TMV;

void registerMask(int, int *, int *);

void inicializacion( int segmentoSizes[], char *filename1, TMV *mv);

void iniciaMV(TMV *,int segmentoSizes[]);

void leeDosBytes(int *, FILE *);

int readHeader(int [], char *,TMV *mv);

void cargaCodigo(TMV *, FILE *, int );

char instruccionActual(TMV);

int instruccionValida(char);

int validDirection(TMV mv, int memoryOp);

int validIP(TMV mv);

int validJump(TMV mv, int salto);

void readOperand(TMV *, int, int *);

int operandValue(TMV, int, int);

int direccion(TMV, int);



