#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operaciones.h"


void procesaInstruccion(TMV *mv, void (*operaciones[])());
void reportError(int error);

int main(int argc, char *argv[]){
  TMV mv;
  int programSize = 0, valido,d;
  void (*operaciones[27])() = {MOV, ADD, SUB, SWAP, MUL, DIV, CMP, SHL, SHR, AND, OR, XOR,
                               RND, NULL, NULL, NULL, SYS, JMP, JZ, JP, JN, JNZ, JNP, JNN, LDL, LDH, NOT};

  // verifica  q el archivo sea del tipo correcto y almacena el tama�o del programa en bytes
  valido = leeHeader(&programSize, argv[1],&mv);

  if (valido){
    // cargo el codigo en la memoria principal
    cargaCodigo(&mv, argv[1], programSize);

    // inicializo registros CS,DS,IP y Tabla de Segmento
    iniciaMV(&mv, programSize);

    // procesa programa hasta encontrar un STOP o que ocurra un error (segmento invalido,div por 0,etc)
    while (((instruccionActual(mv)&0xFF) != 0xFF) && !mv.errorFlag) {
      procesaInstruccion(&mv, operaciones);
      mv.errorFlag = !(segmentoCheck(mv,0,3)); //chequea q el ip apunte dentro de code segment
    }

    if(mv.errorFlag)
        reportError(mv.errorFlag);
    else
        printf("\nSUCCESSFUL EXECUTION :)\n");
  }

  scanf("%d",&d);
  return 0;
}

void procesaInstruccion(TMV *mv, void (*operaciones[])()){
  int A, B, opA, opB, codOp,vA,vB;
  char instruccion = instruccionActual(*mv);

  /*printf("\ninstruccion: ");
 for (int i = 7; i >= 0; i--)
    printf("%d", (instruccion >> i) & 1);
   printf("\n");*/

  codOp = instruccion & 0x1F;
  if( instruccionValida(codOp)){
  // determino tipos de operandos
  opB = ((instruccion & 0xC0) >> 6) & 0x03;
  opA = (instruccion & 0x30) >> 4;

  //IP queda apuntando a la prox instruccion
  mv->registros[5]++;

  //obtengo operandos
  readOperand(mv, opB, &B);
  readOperand(mv, opA, &A);

  //si el operando es de memoria chequeo q no se salga de segmento la direccion a donde tengo q buscar el dato, sino lo doy como valido
  //chequearlo antes de ir a la operacion me facilita no tener q estar chequeandolo adentro de cada operacion
  vA = (opA == 0)?segmentoCheck(*mv,opA,1):1;
  vB = (opB == 0)?segmentoCheck(*mv,opB,1):1;

  if(vA && vB){

  //segun el codigo de operacion, llamo la q corresponda
  if(codOp & 0x10){
   if(codOp>=0x11 && codOp<=0x17)
     jump(B,codOp,mv,operaciones); //aca llamo a los jumps
    else
     operaciones[codOp](B,opB,mv); //operaciones 1 operando
   }else
     operaciones[codOp](A, opA, B, opB, mv); //operaciones de 2 operandos

  }else{
     mv->errorFlag = 1; //segmento invalido
   }

  }else
    mv->errorFlag = 2; //instruccion invalida
}

void reportError(int error)
{
switch(error){
 case 1: printf("INVALID SEGMENT");
        break;
 case 2: printf("INVALID INSTRUCTION");
        break;
 case 3: printf("DIVISION BY ZERO");
        break;
 case 4: printf("INVALID ARCHIVE TYPE");
        break;
 case 5: printf("INVALID PROGRAM SIZE");
        break;
 }
}
