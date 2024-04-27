#include <string.h>
#include <stdio.h>
#include "operaciones.h"


const char* fnNombres[] = {"MOV", "ADD", "SUB", "SWAP", "MUL", "DIV", "CMP", "SHL", "SHR", "AND", "OR", "XOR",
                           "RND", "NULL", "NULL", "NULL", "SYS %", "JMP", "JZ", "JP", "JN", "JNZ", "JNP", "JNN", "LDL", "LDH", "NOT",
                           "NULL","NULL","NULL","STOP","STOP"};

const char* regNombres[] = {"CS","DS","ES","SS","KS","IP","SP","BP","CC","AC",
                          "EAX","AL","AH","AX","EBX","BL","BH","BX","ECX","CL","CH","CX","EDX","DL","DH","DX",
                          "EEX","EL","EH","EX","EFX","FL","FH","FX"};

const char* cantCelda[] = {'l',' ','w','b'};

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
    char celdas;
    switch(tipo)
    {
        //Memoria
        case 0:
            celdas = cantCelda[(valor>>22)&0x3];
            valor = valor&0x0FFFFF;

            codReg = (valor >>16)& 0xFF;

            if (codReg >= 10)
                codReg = (codReg - 10) * 4 + 10;

            printf("%c[%s",celdas, regNombres[codReg]);
            valor &= 0xFFFF;
            valor = valor<<16>>16;

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

  //muestro en hexa la info de los operandos y los guardo
    for (i = 0; i < operadorSize; ++i)
     {
        printf("%02X ", (mv->memoria[ip] & 0xFF));
       *operador = (*operador << 8) | (mv->memoria[ip++] & 0xFF);
     }
    mv->registros[IP] = ip;

    if(tipo == 1)
    {
        *operador = (*operador)<<16>>16;
	}
}

void fillExtraDis(int extraSpace)
{
    for (int i = 0; i < extraSpace; i++)
        printf("   ");
}



