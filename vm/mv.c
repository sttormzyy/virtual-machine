#include <string.h>
#include "mv.h"

void registerMask(int secReg, int *corr, int *mask) {
    *mask = 0xFFFFFFFF;
	*corr = 0;
	if (secReg) {
		switch (secReg)
		{
		case 1:
			*mask = 0x000000FF;
			break;
		case 2:
			*mask = 0x0000FF00;
			*corr = 1;
			break;
		case 3:
			*mask = 0x0000FFFF;
			break;
		}
	}
}

void iniciaMV(TMV *mv, int programSize) {
	// CS <- 00 00 00 00 | DS <- 00 01 00 00 | IP <- 00 00 00 00
	mv->registros[0] = mv->registros[1] = mv->registros[5] = 0;
	mv->registros[1] = mv->registros[1] | 0x10000;

	// 0| 00    00     PROGRAM_SIZE
	// 1| PROGRAM_SIZE MEMORY_SIZE
	mv->tablaSegmentos[0] = mv->tablaSegmentos[1] = 0;
	mv->tablaSegmentos[0] = programSize;
	mv->tablaSegmentos[1] = (programSize << 16) | (MEMORIA_SIZE - 1);
}

// lee el header, retorna 1 si hay error, 0 si esta todo bien
// nota: podemos agregar mas codigos de error en caso de que querramos diferenciar que es lo que falló
int leeHeader(int *programSize, char *filename) {
	FILE *arch = fopen(filename, "rb");
	char tipoArch[6], version, x, y;

	fread(tipoArch, sizeof(char), 5, arch);

	if (!strcmp(tipoArch, "VMX24")) {
		// lee la version
		fread(&version, sizeof(char), 1, arch);

		// lee el tamaño del programa
        fread(&x, sizeof(char), 1, arch);
        fread(&y, sizeof(char), 1, arch);
        *programSize = ((int)x << 8) | ((int) y);

        // tamaño del programa valido
        if (programSize) {
          printf("%s %d\n", tipoArch, version);
          printf("PROGRAM SIZE: %d\n", *programSize);

          return 0;
        } else {
          return 2;
        }
	} else {
    return 1;
	}
}

void cargaCodigo(TMV *mv, char *filename, int programSize)
{
	FILE *arch = fopen(filename, "rb");

	// salta el header
	fseek(arch, 8, 0);

	fread(mv->memoria, sizeof(char), programSize, arch);

	// muestra por pantalla el codigo en memoria
	for (int h = 0; h < programSize; h++) {
		printf("%d: %X\n", h, mv->memoria[h] & 0xFF); // imprime de a bytes
	}
}

char instruccionActual(TMV mv) {
  return mv.memoria[mv.registros[5]] & 0xFF;
}

void readOperand(TMV *mv, int tipo, int *operador) {
	char operadorSize = (~tipo) & 0x3;
	int i, IP = mv->registros[5];
	*operador = 0;

	// obtengo la información que esta en memoria sobre el operador
	// nota: IP podria ser reemplazado por mv->registros[5] pero es mas legible como IP
	for (i = 0; i < operadorSize; ++i) {
		*operador = *operador << 8 | (mv->memoria[IP++] & 0xFF);
		mv->registros[5] = IP;
	}
}

int operandValue(TMV mv, int operand, int tipo) {
	int dir, valor = operand, i, secReg, mask, codReg, corr;

	switch(tipo) {
		case 0:
			dir = direccion(mv, operand);
			for (i = 0; i < 4; ++i) {
				valor = (valor << i * 8) + mv.memoria[dir];
			}
			break;
		case 2:
			secReg = (operand >> 4) & 0x3;
			registerMask(secReg, &corr, &mask);
			codReg = operand & 0xF;
			valor = mv.registros[codReg] & mask;
			if (secReg == 2)
				valor = valor >> 8;
		    printf("%d %d %d %d %d\n", operand, secReg, valor, codReg, mv.registros[codReg]);
			break;
	}
	return valor;
}

int direccion(TMV mv, int memoryData) {
	char codReg = (memoryData >> 16) & 0xFF;
	int offset = (memoryData & 0xFFFF) + (mv.registros[codReg] & 0xFFFF);
	int inicioSegmento = mv.tablaSegmentos[(mv.registros[codReg] >> 16) & 0x1] & 0xFFFF;
	return inicioSegmento + offset;
}
