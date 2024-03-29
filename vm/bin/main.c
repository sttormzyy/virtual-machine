#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operaciones.h"


void procesaInstruccion(TMV *mv, void (*operaciones[23])());

int main(int argc, char *argv[]) {
  TMV mv;
  int programSize = 0, valido,d;
  void (*operaciones[24])() = {MOV, ADD, SUB, SWAP, MUL, DIV, CMP, SHL, SHR, AND, OR, XOR,
                               RND, SYS, JMP, JZ, JP, JN, JNZ, JNP, JNN, LDL, LDH, NOT};

  // verifica  q el archivo sea del tipo correcto y almacena el tamaño del programa en bytes
  valido = leeHeader(&programSize, argv[1]);

  if (!valido) {
    // cargo el codigo en la memoria principal
    cargaCodigo(&mv, argv[1], programSize);

    // inicializo registros CS,DS,IP y Tabla de Segmento
    iniciaMV(&mv, programSize);

    // procesa programa hasta encontrar un STOP
    // nota: tambien otra condicion de salida tendria que ser que salga del programa, que el ip salga del segmento de codigo
    while (instruccionActual(mv) != 0xFF) {
      procesaInstruccion(&mv, operaciones);
    }
  }

  scanf("%d",&d);
  return 0;
}

void procesaInstruccion(TMV *mv, void (*operaciones[23])()) {
  int A, B, opA, opB, codOp;
  char instruccion = instruccionActual(*mv);

  printf("\ninstruccion: ");
for (int i = 7; i >= 0; i--){
    printf("%d", (instruccion >> i) & 1);
    }
  printf("\n");

  // determino tipos de operandos y codigo de operacion
  opB = ((instruccion & 0xC0) >> 6) & 0x03;
  opA = (instruccion & 0x30) >> 4;
  codOp = instruccion & 0x1F;
  printf("opB: %X | opA: %X | codOp: %x\n",opB, opA, codOp);
  mv->registros[5]++;

  // obtengo valor operandos y acomodo el IP a siguiente instruccion
  // si el operando A es ninguno no busco segundo valor de operando
  readOperand(mv, opB, &B);
  if (opA != 3)
    readOperand(mv, opA, &A);

  // segun el codigo de operacion, llamo la q corresponda (distingo entre ops de 1 y 2 operandos)
  if ((codOp & 0x10) != 0)
    operaciones[codOp - 3](B, opB, A, opB, mv);
  else
    operaciones[codOp](A, opA, B, opB, mv);
}
