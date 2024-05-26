#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mv.h"


void iniciaMV(TMV *mv, int segmentoSizes[])
{
	int posicionTablaSegmento = 0, i, memoriaSizeControl = 0;
	mv->memoria = (char *) malloc(mv->memorySize);

	//carga los registros CS, DS, ES, SS, KS con los valores del header
	if (segmentoSizes[KS])
    {
		mv->registros[KS] = 0;
		mv->tablaSegmentos[posicionTablaSegmento] = segmentoSizes[KS];
		memoriaSizeControl += segmentoSizes[KS];
		posicionTablaSegmento++;
	} else
		mv->registros[KS] = -1;

	for (i = 0; i < 4; ++i)
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

	//inicializo SP e IP con el offset correspondiente
	mv->registros[IP] = (mv->registros[CS] & 0xFFFF0000) | (segmentoSizes[IP] & 0xFFFF);
    mv->registros[SP] = mv->registros[SS] | segmentoSizes[SS];
	mv->errorFlag = 0;
	mv->modo = !DEBUG;
}

void leeDosBytes(int *variableRetorno, FILE *arch)
{
	char size[3];
    fread(size, sizeof(char), 2, arch);
    *variableRetorno = ((size[0] & 0xFF) << 8) | (size[1] & 0xFF);
    *variableRetorno = abs(*variableRetorno);
}

void inicializacion( int segmentoSizes[], char *filename1, TMV *mv)
{
	FILE *arch;
	char tipoArch[6], version;
	int d, i, memoriaSizeControl = 0;

	if(strstr(filename1,".vmx") != NULL) //VMX
    {
        arch = fopen(filename1, "rb");

        fread(tipoArch, sizeof(char), 5, arch);
        fread(&version, sizeof(char), 1, arch);

        switch(version)
        {
            case 1: //VMX V1
                leeDosBytes(&(segmentoSizes[CS]), arch);
                segmentoSizes[DS] = mv->memorySize - segmentoSizes[CS];
                segmentoSizes[IP] = segmentoSizes[KS] = segmentoSizes[ES] = segmentoSizes[SS] = 0;
                fseek(arch, 8, SEEK_SET);
                break;
            case 2: //VMX V2
                for (i = 0; i < 6; ++i)
                {
                    leeDosBytes(&(segmentoSizes[i]), arch);
                    memoriaSizeControl += segmentoSizes[i];
                }
                fseek(arch, 18, SEEK_SET);
                break;
        }

        if (memoriaSizeControl<=mv->memorySize)
        {
            iniciaMV(mv, segmentoSizes);
            cargaCodigo(mv, arch, segmentoSizes[CS]);
        }else
            mv->errorFlag=5; //memoria insuficiente

	}else //VMI
	    if(strstr(filename1, ".vmi") != NULL)
        {
            arch = fopen(filename1, "rb");

            fread(tipoArch, sizeof(char), 5, arch);
            fread(&version, sizeof(char), 1, arch);

            leeDosBytes(&(mv->memorySize), arch);
            mv->memorySize = mv->memorySize * 0x400;
            mv->memoria = (char*)malloc(mv->memorySize);

            for(int i=0; i<16; i++)
            {
                fread(&(mv->registros[i]), sizeof(char), 4, arch);
                invierteBytes(&(mv->registros[i]));
            }


           for(int i=0; i<8; i++)
           {
               fread(&(mv->tablaSegmentos[i]), sizeof(char), 4, arch);
               invierteBytes(&(mv->tablaSegmentos[i]));
           }

            fread(mv->memoria, sizeof(char), mv->memorySize, arch);
            fclose(arch);
       }else
           mv->errorFlag = 4; //tipo de archivo invalido que no soporta la MV
}

void cargaCodigo(TMV *mv, FILE *arch, int programSize)
{
	if (mv->registros[KS] != -1)
    {
		fread(mv->memoria + (mv->tablaSegmentos[0] & 0xFFFF), sizeof(char), programSize, arch);
		fread(mv->memoria, sizeof(char), mv->tablaSegmentos[0] & 0xFFFF, arch);
	} else
		fread(mv->memoria, sizeof(char), programSize, arch);
}

char instruccionActual(TMV mv)
{
    int posTabla = (mv.registros[CS]>>16)&0xF;
    return mv.memoria[((mv.tablaSegmentos[posTabla]>>16)&0xFFFF)+(mv.registros[IP]&0xFFFF)] & 0xFF;
}

int instruccionValida(char codOp)
{
    return (codOp>=0 && codOp <=0xC) || (codOp>=0x10 && codOp<=0x1F);
}

int validDirection(TMV mv, int memoryOp)
{
    int dir = direccion(mv,memoryOp);
    int registro = (memoryOp >> 16) & 0xF;
    int posTablaSegmento = (mv.registros[registro] >> 16)  & 0xF;
    int comienzo = (mv.tablaSegmentos[posTablaSegmento]>>16) & 0xFFFF;
    int tope = comienzo + (mv.tablaSegmentos[posTablaSegmento]&0xFFFF);
    return (dir>=comienzo) && (dir<tope);
}

int validIP(TMV mv)
{
    int posCS = (mv.registros[KS]!=-1)?1:0;
    return (mv.registros[IP]&0xFFFF) < (mv.tablaSegmentos[posCS]&0xFFFF);

}

int validJump(TMV mv, int salto)
{
    int posCS = (mv.registros[KS]!=-1)?1:0;
    return salto < (mv.tablaSegmentos[posCS]&0xFFFF);
}

void readOperand(TMV *mv, int tipo, int *operador)
{
	char operadorSize = (~tipo) & 0x3;
	int i, ip = (((mv->tablaSegmentos[mv->registros[CS] >> 16] >> 16) & 0xFFFF) + mv->registros[IP] & 0xFFFF);
	*operador = 0;

	//obtengo la informacion que esta en memoria sobre el operador
	for (i = 0; i < operadorSize; ++i)
    {
		*operador = ((*operador) << 8) | (mv->memoria[ip++] & 0xFF);
	}
	mv->registros[IP] += operadorSize;

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
	int inicioSegmento = (mv.tablaSegmentos[(mv.registros[codReg] >> 16) & 0xF]>>16) & 0xFFFF;
	return inicioSegmento + offsetReg + offsetInst;
}

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

void invierteBytes(int* value)
{
    char bytes[4];
    bytes[3] = *value & 0xFF;
    bytes[2] = (*value >> 8) & 0xFF;
    bytes[1] = (*value >> 16) & 0xFF;
    bytes[0] = (*value >> 24) & 0xFF;

    *value = ((bytes[3]<<24)&0xFF000000) | ((bytes[2]<<16)&0xFF0000) | ((bytes[1]<<8)&0xFF00) | (bytes[0]&0xFF);
}
