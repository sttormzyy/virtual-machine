#include "constantes.h"

#define MEMORIA_SIZE 16384

typedef struct {
	char* memoria;
	char version;
	int registros[16];
	int tablaSegmentos[8];
	int errorFlag;
} TMV;

void registerMask(int, int *, int *);

void iniciaMV(TMV *, int);

void leeDosBytes(int *, FILE *); 

int readHeader(int [], char *,TMV *mv);

void cargaCodigo(TMV *, char *, int );

char instruccionActual(TMV);

int instruccionValida(char);

int segmentoCheck(TMV , int, int);

void readOperand(TMV *, int, int *);

int operandValue(TMV, int, int);

int direccion(TMV, int);



