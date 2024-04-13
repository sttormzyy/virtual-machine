#include <string.h>
#include <stdlib.h>
#include "mv.h"

void registerMask(int secReg, int *corr, int *mask)
{
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

void iniciaMV(TMV *mv, int programSize)
{
	// CS <- 00 00 00 00 | DS <- 00 01 00 00 | IP <- 00 00 00 00
	mv->registros[0] = mv->registros[1] = mv->registros[5] = 0;
	mv->registros[1] = mv->registros[1] | 0x10000;

	// 0| 00    00     PROGRAM_SIZE
	// 1| PROGRAM_SIZE MEMORY_SIZE-PROGRAM_SIZE
	mv->tablaSegmentos[0] = mv->tablaSegmentos[1] = 0;
	mv->tablaSegmentos[0] = programSize;
	mv->tablaSegmentos[1] = (programSize << 16) | (MEMORIA_SIZE - programSize);

	mv->errorFlag=0;
}

// lee el header, retorna 1 si hay error, 0 si esta todo bien
// nota: podemos agregar mas codigos de error en caso de que querramos diferenciar que es lo que fall�
int leeHeader( int *programSize, char *filename, TMV *mv)
{
	FILE *arch = fopen(filename, "rb");
	char tipoArch[6], version, size[3];

	fread(tipoArch, sizeof(char), 5, arch);

	if (!strcmp(tipoArch, "VMX24")) {
		// lee la version
		fread(&version, sizeof(char), 1, arch);

		// lee el tama�o del programa
        fread(size, sizeof(char), 2, arch);

        *programSize = (size[0] << 8) | size[1];
        *programSize = abs(*programSize);

        // valido tama�o del programa
        if (*programSize!=0  && *programSize<=MEMORIA_SIZE) {
         printf("%s %d\n", tipoArch, version);
         printf("PROGRAM SIZE: %d\n", *programSize);
         fclose(arch);
         return 1;
        }else {
         mv->errorFlag=5; //o el tamanio es 0 o es mayor q la memoria
         fclose(arch);
         return 0;
        }
	}else {
     mv->errorFlag = 4; //tipo de archivo invalido no es de la vmx
     fclose(arch);
     return 0;
	}
}

void cargaCodigo(TMV *mv, char *filename, int programSize)
{
	FILE *arch = fopen(filename, "rb");

	// salta el header
	fseek(arch, 8, 0);

	fread(mv->memoria, sizeof(char), programSize, arch);

	// muestra por pantalla el codigo en memoria
	for(int h=0;h<programSize;h++){
        printf("%d:",h);
        for (int i = 7; i >= 0; i--)
       printf("%d", (mv->memoria[h] >> i) & 1);
       printf("\n");
    }

    fclose(arch);
}

char instruccionActual(TMV mv)
{
  return mv.memoria[mv.registros[5]] & 0xFF;
}

//verifica que el codigo de operacion exista
int instruccionValida(char codOp)
{
 return (codOp>=0 && codOp <=0xC) || (codOp>=0x10 && codOp<=0x1A) || (codOp==0x1F);
}

//verifica que el operando de memoria o los jumps no se pasen de segmento
int segmentoCheck(TMV mv,int oprnd,int tipo)
{
int posTabla;

switch(tipo){
 case 1: //verifica q el operando de memoria no se salga de segmento
  posTabla = (mv.registros[(oprnd>>16) & 0xF] >> 16) & 0x1;
  return direccion(mv,oprnd) <= (((mv.tablaSegmentos[posTabla]>>16) & 0xFFFF) + (mv.tablaSegmentos[posTabla]&0xFFFF));
  break;

 case 2: // verifica que el jump no se salga de segmento
  return oprnd <= (mv.tablaSegmentos[0]&0xFFFF);
  break;

 case 3: //verifica que el IP no se salga de segmento
  return (mv.registros[5]&0xFFFF) <= (mv.tablaSegmentos[0]&0xFFFF);
 }
 return 0;
}

void readOperand(TMV *mv, int tipo, int *operador)
{
	char operadorSize = (~tipo) & 0x3;
	int i, IP = mv->registros[5];
	short int aux;
	*operador = 0;

	// obtengo la informaci�n que esta en memoria sobre el operador
	// nota: IP podria ser reemplazado por mv->registros[5] pero es mas legible como IP
	for (i = 0; i < operadorSize; ++i) {
		*operador = (*operador << 8) | (mv->memoria[IP++] & 0xFF);
	}
	mv->registros[5] = IP;

    //puede parecer raro, pero vi que si haciamos mov a un registro un negativo, ponele MOV EBX,-34 no se acarreaba el signo en todo
    // el registro xq inicializamos arriba el oeprador en 0 entonces los 16 bits mas significativos siempre estan en 0, entonces lo q hago
    // es pasar el valor a un short int q tiene 16 bits y lo vuelvo a poner en operador entonces si era negativo expande el signo solo
    // solo va "servir" si es negativo y queremos guardarlo en un registro completo, si fuera guardar en BX en vez de EBX lo q teniamos funca
	if(tipo == 1){
        aux = *operador;
        *operador = aux;
	}
}

int operandValue(TMV mv, int operand, int tipo)
{
	int dir, valor = operand, i, secReg, mask, codReg, corr;
	short int aux1;
	char aux;

	switch(tipo) {
		case 0:
		    valor=0;
			dir = direccion(mv, operand);
			for (i = 3; i >= 0; i--) {
				valor |= (mv.memoria[dir++]&0xFF)<<8*i;
			}
			//printf("valor memoria %d dir %d\n",valor,dir-4);
			break;
		case 2:
			secReg = (operand >> 4) & 0x3;
			registerMask(secReg, &corr, &mask);
			codReg = operand & 0xF;
			valor = mv.registros[codReg] & mask;
            valor = valor >> (8*corr);

            //acarreo signo si no tomo el registro completo sirve si hago MOV de registro a memoria nomas pero es necesario
            //ponele q hice MOV BL,-3 y quiero hacer MOV [15],BL como va guardar 4 celdas de 8 bits en memoria y solo tome un byte de registro B
            // necesito expandirle el signo asi se guarda en memoria el -3 y no otra cosa
            if(secReg == 1 || secReg == 2 ){
               aux = valor;
               valor = aux;
            }
            if(secReg == 3){
                aux1 = valor;
                valor = aux1;
            }
			break;
	}
	return valor;
}

int direccion(TMV mv, int memoryOp)
{
	char codReg = (memoryOp >> 16) & 0xFF;
	int offset = (memoryOp & 0xFFFF) + (mv.registros[codReg] & 0xFFFF);
	int inicioSegmento = (mv.tablaSegmentos[(mv.registros[codReg] >> 16) & 0x1]>>16) & 0xFFFF;
	return inicioSegmento + offset;
}

void disassembler(TMV mv, int programSize) {
  int ip = mv.registros[5];
  mv.errorFlag = 0;

  while (mv.registros[5] < programSize && !mv.errorFlag) {
    pasoDis(&mv, instruccionActual(mv));
  }

  mv.registros[5] = ip;
}

void pasoDis(TMV *mv, char instruccion) {
  char fnNombres[32][5] = {"MOV", "ADD", "SUB", "SWAP", "MUL", "DIV", "CMP", "SHL", "SHR", "AND", "OR", "XOR",
                           "RND", "NULL", "NULL", "NULL", "SYS", "JMP", "JZ", "JP", "JN", "JNZ", "JNP", "JNN", "LDL", "LDH", "NOT",
                           "NULL","NULL","NULL","STOP","STOP"};
  int codOp = instruccion & 0x1F;
  int opB, opA, B, A, i;
  if (instruccionValida(codOp)) {
    printf("[%04X] ", mv->registros[5]);
    // determino tipos de operandos
    opB = ((instruccion & 0xC0) >> 6) & 0x03;
    opA = (instruccion & 0x30) >> 4;

    for (i = 1; i >= 0; --i)
      printf("%02X ", instruccion >> (i * 4) & 0xFF);

    // IP queda apuntando a la prox instruccion
    (mv->registros[5])++;

    // obtengo operandos
    readOperandDis(mv, opB, &B);
    readOperandDis(mv, opA, &A);
    fillExtraDis(opB, opA);

    printf("| %s ", fnNombres[codOp]);

    if (codOp & 0x10 && (codOp >= 0x11 && codOp <= 0x17))
    {
      mostrarOp(opB, B);
    }
    else if (codOp >= 0x00 && codOp <= 0x0C)
    {
      mostrarOp(opA, A);
      putchar(',');
      mostrarOp(opB, B);
    }

    putchar('\n');
  }
  else
    mv->errorFlag = 2; // instruccion invalida
}

int checkParam(int argc, char *argv[], char strCmp[]) {
	int i = 0;

	while( argc > i && strcmp(argv[i], strCmp)) {
		i++;
	}

	if (i < argc)
		return 1;
	return 0;
}

void mostrarOp(int tipo, int valor) {
  char regNombres[34][4] = {"CS","DS","XXX","XXX","XXX","IP","XXX","XXX","CC","AC",
                          "EAX","AL","AH","AX","EBX","BL","BH","BX","ECX","CL","CH","CX","EDX","DL","DH","DX",
                          "EEX","EL","EH","EX","EFX","FL","FH","FX"};
  int secReg, codReg;

  switch(tipo) {
  // Memoria
  case 0:
    secReg = (valor >> 4) & 0x3;
    codReg = valor & 0xF;

    if (codReg >= 10) {
      codReg = (codReg - 10) * 4 + 10;
    }
    printf("[%s", regNombres[secReg + codReg]);
    printf(" + %4d]", valor & 0xFFFF);
    break;
  // Inmediato
  case 1:
    printf("%3d", valor);
    break;
  // Registro
  case 2:
    secReg = (valor >> 4) & 0x3;
    codReg = valor & 0xF;

    if (codReg >= 10) {
      codReg = (codReg - 10) * 4 + 10;
    }
    printf("%s", regNombres[secReg + codReg]);
  }
}

void readOperandDis(TMV *mv, int tipo, int *operador)
{
  char operadorSize = (~tipo) & 0x3;
  int i, IP = mv->registros[5];
  short int aux;
  *operador = 0;

  // obtengo la informaci�n que esta en memoria sobre el operador
  // nota: IP podria ser reemplazado por mv->registros[5] pero es mas legible como IP
  for (i = 0; i < operadorSize; ++i) {
    printf("%02X ", (mv->memoria[IP] & 0xFF));
    *operador = (*operador << 8) | (mv->memoria[IP++] & 0xFF);
  }
  mv->registros[5] = IP;

  // puede parecer raro, pero vi que si haciamos mov a un registro un negativo, ponele MOV EBX,-34 no se acarreaba el signo en todo
  //  el registro xq inicializamos arriba el oeprador en 0 entonces los 16 bits mas significativos siempre estan en 0, entonces lo q hago
  //  es pasar el valor a un short int q tiene 16 bits y lo vuelvo a poner en operador entonces si era negativo expande el signo solo
  //  solo va "servir" si es negativo y queremos guardarlo en un registro completo, si fuera guardar en BX en vez de EBX lo q teniamos funca
  if (tipo == 1)
  {
    aux = *operador;
    *operador = aux;
  }
}

void fillExtraDis(int opB, int opA) {
    int extraSpace = opA + opB, i;
    for (i = 0; i < extraSpace; i++)
      printf("   ");
}
