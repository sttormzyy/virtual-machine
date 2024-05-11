#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mv.h"

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

void iniciaMV(TMV *mv, int segmentoSizes[])
{
	int posicionTablaSegmento = 0, i, memoriaSizeControl = 0;
	mv->memoria = (char *) malloc(MEMORIA_SIZE);
	
	// carga los registros CS, DS, ES, SS, KS con los valores del header
	if (segmentoSizes[KS]) {
		mv->registros[KS] = 0;
		mv->tablaSegmentos[posicionTablaSegmento] = segmentoSizes[KS];
		memoriaSizeControl += segmentoSizes[KS];
		posicionTablaSegmento++;
	} else
		mv->registros[KS] = -1;

	for (i = 0; i < 5; ++i) 
	{
		if (segmentoSizes[i] != 0) 
		{
			mv->registros[i] = posicionTablaSegmento << 16;
			mv->tablaSegmentos[posicionTablaSegmento] = (memoriaSizeControl << 16) | (segmentoSizes[i] & 0xFFFF);
			memoriaSizeControl += segmentoSizes[i] & 0xFFFF;
			posicionTablaSegmento++;
		} else
			mv->registros[i] = -1;
	}

	mv->registros[IP] = (mv->registros[CS] & 0xFFFF0000) | segmentoSizes[IP];
	mv->errorFlag=0;
}

void leeDosBytes(int *variableRetorno, FILE *arch) 
{
	char size[3];
    fread(size, sizeof(char), 2, arch);
    variableRetorno = ((size[0] & 0xFF) << 8) | (size[1] & 0xFF);
    variableRetorno = abs(variableRetorno);
}

int readHeader( int segmentoSizes[], char *filename, TMV *mv)
{
	FILE *arch = fopen(filename, "rb");
	char tipoArch[6], size[3];
	int d, i, memoriaSizeControl = 0;


	fread(tipoArch, sizeof(char), 5, arch);


	if (!strcmp(tipoArch, "VMX24"))
    {
		// lee la version
		fread(&(mv->version), sizeof(char), 1, arch);

		// lee el tamanio de los segmentos y el offset del IP
		if (mv->version == 2) {
			for (i = 0; i < 6; ++i) {
				leeDosBytes(&(segmentoSizes[i]), arch);
				memoriaSizeControl += segmentoSizes[i];
			}
		}
		else {
			leeDosBytes(&(segmentoSizes[CS]), arch);
			segmentoSizes[DS] = MEMORIA_SIZE - segmentoSizes[CS];
			segmentoSizes[IP] = segmentoSizes[KS] = segmentoSizes[ES] = segmentoSizes[SS] = 0;
		}

        // valido tamanio del programa
        if (segmentoSizes[0]!=0)
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
	fseek(arch, 18, SEEK_SET);

	if (mv->registros[KS] != -1) {
		fread((char *)((int) mv->memoria + mv->registros[KS] & 0xFFFF), sizeof(char), programSize, arch);
		fread(mv->memoria, sizeof(char), mv->tablaSegmentos[0] & 0xFFFF, arch);
	} else
		fread(mv->memoria, sizeof(char), programSize, arch);

	for(int i=0;i<programSize;i++)
    {
        for(int k=7;k>=0;k--)
            printf("%d",(mv->memoria[i]>>k)&1);
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
	*operador = 0;

	// obtengo la informacion que esta en memoria sobre el operador
	for (i = 0; i < operadorSize; ++i)
    {
		*operador = (*operador << 8) | (mv->memoria[ip++] & 0xFF);
	}
	mv->registros[IP] = ip;

    //expande signo si es inmediato
	if(tipo == 1)
    {
      *operador= (*operador)<<16>>16;
	}
}

int operandValue(TMV mv, int operand, int tipo)
{
	int dir, valor = operand, i, secReg, mask, codReg, corr, cantCelda;

	switch(tipo)
	{
	    //Memoria
		case 0:
		    valor=0;
		    cantCelda = (~((operand>>22)&0x3))&0x3;
			dir = direccion(mv, operand);
			for (i = cantCelda; i >= 0; i--)
            {
				valor |= (mv.memoria[dir++]&0xFF)<<8*i;
			}
			valor = valor<<(24-cantCelda*8)>>(24-cantCelda*8);
			break;
        //Registro
		case 2:
			secReg = (operand >> 4) & 0x3;
			registerMask(secReg, &corr, &mask);
			codReg = operand & 0xF;
			valor = mv.registros[codReg] & mask;
            valor = valor >> (8*corr);

            //expando signo si no tomo el registro completo
            if(secReg == 1 || secReg == 2 )
            {
               valor = valor<<24>>24;
            }
            if(secReg == 3)
            {
                valor = valor<<16>>16;
            }
			break;
	}
	return valor;
}

int direccion(TMV mv, int memoryOp)
{
    memoryOp = memoryOp&0x0FFFFF;
	char codReg = (memoryOp >> 16) & 0xFF;
    short int offsetReg = (mv.registros[codReg] & 0xFFFF);
	short int offsetInst = (memoryOp & 0xFFFF);
	int inicioSegmento = (mv.tablaSegmentos[(mv.registros[codReg] >> 16) & 0x1]>>16) & 0xFFFF;
	return inicioSegmento + offsetReg + offsetInst;
}


