#include <string.h>
#include <stdlib.h>
#include "mv.h"
#include "constantes.h"


void registerMask(int secReg, int *corr, int *mask)
{
    *mask = 0xFFFFFFFF;
	*corr = 0;
	if (secReg)
    {
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
	mv->registros[1] |= 0x10000;

	// 0| 00    00     PROGRAM_SIZE
	// 1| PROGRAM_SIZE MEMORY_SIZE-PROGRAM_SIZE
	mv->tablaSegmentos[0] = mv->tablaSegmentos[1] = 0;
	mv->tablaSegmentos[0] = programSize;
	mv->tablaSegmentos[1] = (programSize << 16) | (MEMORIA_SIZE - programSize);

	mv->errorFlag=0;
}


int readHeader( int *programSize, char *filename, TMV *mv)
{
	FILE *arch = fopen(filename, "rb");
	char tipoArch[6], version, size[3];

	fread(tipoArch, sizeof(char), 5, arch);

	if (!strcmp(tipoArch, "VMX24"))
    {
		// lee la version
		fread(&version, sizeof(char), 1, arch);

		// lee el tamanio del programa
        fread(size, sizeof(char), 2, arch);
        *programSize = ((size[0] & 0xFF) << 8) | (size[1] & 0xFF);
        *programSize = abs(*programSize);

        // valido tamanio del programa
        if (*programSize!=0  && *programSize<=MEMORIA_SIZE)
        {
            fclose(arch);
            return 1;
        }else
        {
            mv->errorFlag=5; // el tamanio es 0 o es mayor q el de la memoria
            fclose(arch);
            return 0;
        }
	}else
	{
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
    int posTabla, dir, tope, comienzo;

    switch(tipo)
    {
        case 1: //verifica q el operando de memoria no se salga de segmento
            dir = direccion(mv,oprnd);
            tope = (((mv.tablaSegmentos[1]>>16) & 0xFFFF) + (mv.tablaSegmentos[1]&0xFFFF));
            comienzo = (mv.tablaSegmentos[1]>>16) & 0xFFFF;
            return  (dir >= comienzo) && (dir <= tope);
        break;

        case 2: //verifica que el jump no se salga de segmento
            return oprnd <= (mv.tablaSegmentos[0]&0xFFFF);
        break;

        case 3: //verifica que el IP no se salga de segmento
            return (mv.registros[IP]&0xFFFF) < (mv.tablaSegmentos[0]&0xFFFF);
   }
 return 0;
}

void readOperand(TMV *mv, int tipo, int *operador)
{
	char operadorSize = (~tipo) & 0x3;
	int i, ip = mv->registros[IP];
	short int aux;
	*operador = 0;

	// obtengo la informacion que esta en memoria sobre el operador
	for (i = 0; i < operadorSize; ++i)
    {
		*operador = (*operador << 8) | (mv->memoria[ip++] & 0xFF);
	}
	mv->registros[IP] = ip;

    //vi que si haciamos mov a un registro un inmediato negativo, ponele MOV EBX,-34 no se acarreaba el signo en todo
    // el registro xq inicializamos arriba el operador en 0 entonces los 16 bits mas significativos siempre estan en 0, entonces lo q hago
    // es pasar el valor a un short int q tiene 16 bits y lo vuelvo a poner en operador entonces si era negativo expande el signo automaticamente
    // solo va "servir" si es negativo y queremos guardarlo en un registro completo, si fuera guardar en BX en vez de EBX lo q teniamos funciona
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

	switch(tipo)
	{
	    //Memoria
		case 0:
		    valor=0;
			dir = direccion(mv, operand);
			for (i = 3; i >= 0; i--)
            {
				valor |= (mv.memoria[dir++]&0xFF)<<8*i;
			}
			break;
        //Registro
		case 2:
			secReg = (operand >> 4) & 0x3;
			registerMask(secReg, &corr, &mask);
			codReg = operand & 0xF;
			valor = mv.registros[codReg] & mask;
            valor = valor >> (8*corr);

            //acarreo signo si no tomo el registro completo sirve si hago MOV de registro a memoria nomas pero es necesario
            //ponele q hice MOV BL,-3 y quiero hacer MOV [15],BL como va guardar 4 celdas de 8 bits en memoria y solo tome un byte de registro B
            // necesito expandirle el signo asi se guarda en memoria el -3 y no otra cosa
            if(secReg == 1 || secReg == 2 )
            {
               aux = valor;
               valor = aux;
            }
            if(secReg == 3)
            {
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
    short int offsetReg = (mv.registros[codReg] & 0xFFFF);
	short int offsetInst = (memoryOp & 0xFFFF);
	int inicioSegmento = (mv.tablaSegmentos[(mv.registros[codReg] >> 16) & 0x1]>>16) & 0xFFFF;
	return inicioSegmento + offsetReg + offsetInst;
}


// DISASSEMBLER

void disassembler(TMV mv, int programSize)
{
    int ip = mv.registros[IP];

    while (mv.registros[IP] < programSize && !mv.errorFlag)
    {
        pasoDis(&mv, instruccionActual(mv));
    }
    printf("\n\n");
    mv.registros[IP] = ip;
}

void pasoDis(TMV *mv, char instruccion) {
    int codOp = instruccion & 0x1F;
    int opB, opA, B, A, i;

    if (instruccionValida(codOp))
    {
        printf("[%04X] ", mv->registros[IP]);

    //determino tipos de operandos
        opB = ((instruccion & 0xC0) >> 6) & 0x03;
        opA = (instruccion & 0x30) >> 4;


    //muestro en hexa la instruccion
        printf("%02X ", instruccion & 0xFF);

       (mv->registros[IP])++;

    //muestro operandos en hexa y obtengo su valor
       OperandDis(mv, opB, &B);
       OperandDis(mv, opA, &A);
       fillExtraDis(opB+opA);

       printf("| %s ", fnNombres[codOp]);

       if ((codOp & 0x10) && (codOp != 0x1F))
      {
          mostrarOp(opB, B);
      }
      else if (codOp >= 0x00 && codOp <= 0x0C)
      {
          mostrarOp(opA, A);
          putchar(',');
          putchar(' ');
          mostrarOp(opB, B);
      }

      putchar('\n');
   }
   else
       mv->errorFlag = 2; // instruccion invalida
}

int checkParam(int argc, char *argv[], char strCmp[])
{
	int i = 0;

	while( argc > i && strcmp(argv[i], strCmp)) {
		i++;
	}

	if (i < argc)
		return 1;
	return 0;
}

void mostrarOp(int tipo, int valor)
{
    int secReg, codReg;
    short int corrSigno;
    switch(tipo)
    {
        //Memoria
        case 0:
            codReg = (valor >>16)& 0xFF;

            if (codReg >= 10)
                codReg = (codReg - 10) * 4 + 10;

            printf("[%s", regNombres[codReg]);
            valor &= 0xFFFF;
            corrSigno = valor;
            valor = corrSigno;

            if(valor) {
                if (valor > 0)
                    printf("+%d]", valor);
                else
                    printf("%d]", valor);
            }
            else
                printf("]");
        break;

        //Inmediato
        case 1:
            printf("%*d",(valor<10)?1:3, valor);
        break;

        //Registro
        case 2:
            secReg = (valor >> 4) & 0x3;
            codReg = valor & 0xF;

            if(codReg >= 10)
                codReg = (codReg - 10) * 4 + 10;

            printf("%s", regNombres[secReg + codReg]);
    }
}

void OperandDis(TMV *mv, int tipo, int *operador)
{
    char operadorSize = (~tipo) & 0x3;
    int i, ip = mv->registros[IP];
    *operador = 0;
    short int aux;
  //muestro en hexa la info de los operandos y los guardo
    for (i = 0; i < operadorSize; ++i)
     {
        printf("%02X ", (mv->memoria[ip] & 0xFF));
       *operador = (*operador << 8) | (mv->memoria[ip++] & 0xFF);
     }
    mv->registros[IP] = ip;

    if(tipo == 1){
        aux = *operador;
        *operador = aux;
	}
}

void fillExtraDis(int extraSpace)
{
    for (int i = 0; i < extraSpace; i++)
        printf("   ");
}
