#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "operaciones.h"


const char* fnNombres[] = {"MOV", "ADD", "SUB", "SWAP", "MUL", "DIV", "CMP", "SHL", "SHR", "AND", "OR", "XOR",
                           "RND", "NULL", "NULL", "NULL", "SYS", "JMP", "JZ", "JP", "JN", "JNZ", "JNP", "JNN", "LDL", "LDH", "NOT",
                           "PUSH","POP","CALL","RET","STOP"};

const char* regNombres[] = {"CS","DS","ES","SS","KS","IP","SP","BP","CC","AC",
                          "EAX","AL","AH","AX","EBX","BL","BH","BX","ECX","CL","CH","CX","EDX","DL","DH","DX",
                          "EEX","EL","EH","EX","EFX","FL","FH","FX"};

const char* cantCelda[] = {'l',' ','w','b'};

void disassembler(TMV mv, int programSize)
{
    int i,ip = (mv.tablaSegmentos[mv.registros[CS] >> 16] >> 16) & 0xFFFF;

    mostrarConstantes(mv);

    while (ip < programSize && !mv.errorFlag)
    {
        pasoDis(&mv, mv.memoria[ip]&0xFF, &ip);
    }
    printf("\n\n");
}

void pasoDis(TMV *mv, char instruccion, int *ip) {
    int codOp = instruccion & 0x1F;
    int opB, opA, B, A, i;

    if (instruccionValida(codOp))
    {
        if(*ip == (((mv->tablaSegmentos[mv->registros[CS] >> 16] >> 16) & 0xFFFF)+mv->registros[IP]&0xFFFF))
            putchar('>');
        else
            putchar(' ');

        printf("[%04X] ", *ip);

    //determino tipos de operandos
        opB = ((instruccion & 0xC0) >> 6) & 0x03;
        opA = (instruccion & 0x30) >> 4;


    //muestro en hexa la instruccion
        printf("%02X ", instruccion & 0xFF);

      (*ip)++;

    //muestro operandos en hexa y obtengo su valor
       OperandDis(mv, opB, &B, ip);
       OperandDis(mv, opA, &A, ip);
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

	while( argc > i && !strstr(argv[i], strCmp)) {
		i++;
	}

	if (i < argc)
		return i;
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

void OperandDis(TMV *mv, int tipo, int *operador, int *ip)
{
    char operadorSize = (~tipo) & 0x3;
    int i;
    *operador = 0;

  //muestro en hexa la info de los operandos y los guardo
    for (i = 0; i < operadorSize; ++i)
     {
        printf("%02X ", (mv->memoria[*ip] & 0xFF));
       *operador = (*operador << 8) | (mv->memoria[(*ip)++] & 0xFF);
     }


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


void mostrarConstantes(TMV mv)
{
    int i = 0, j = 0, k = 0, offsetAlprim=0;
    if(mv.registros[KS]!=-1)
    {
      while(i < mv.tablaSegmentos[0])
      {
          printf(" [%04X] ",i);
          k=0;
          while(mv.memoria[i]!='\0')
          {
            if (k < 6)
                printf("%02X ",mv.memoria[i]);
            else if (k == 6)
                printf(".. ");
            k++;
            i++;
          }
          if (k <= 6 && mv.memoria[i] == '\0') {
            printf("00 ");
            k++;
          }
          while (k <= 6) {
            printf("   ");
            k++;
          }
          i++;
          printf("| ");
          j = offsetAlprim;
          putchar('"');
          while (*(mv.memoria + j) != '\0') {
            printf("%c", isprint(*(mv.memoria + j))?*(mv.memoria + j):'.');
            j++;
          }
          putchar('"');
          putchar('\n');
          offsetAlprim = i;
      }
    }
}


