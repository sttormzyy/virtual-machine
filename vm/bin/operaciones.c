#include "operaciones.h"


//modifico los bits N,Z de acuerdo al resultado de la ultima operacion logico/matematica
void NZ(TMV *mv, int resultado){

mv->registros[8] &= 0x3FFFFFFF;

if(resultado<0){
   mv->registros[8] |= 0x80000000;
}

if(!resultado){
   mv->registros[8] |= 0x40000000;
}

}

void MOV(int A, int opA, int B, int opB, TMV *mv) {
int secReg, mask, codReg, dir, i;
int valB = operandValue(*mv, B, opB), corr;

//esto es para ver por pantalla si hace mov a que registro o a q pos de memoria
if(opA==2){
 printf("MOV %x %d ", A&0xF, valB);
 for(int k=7;k>=0;k--){
  printf("%d",(A>>k)&0x1);
  }
 printf("\n");
 }else{
  printf("MOV %d %d\n",  direccion(*mv, A), valB);
 }

switch(opA) {
 //MOV a memoria
 case 0:
  dir = direccion(*mv, A);
  for (i = 0; i < 4; ++i) {
   mv->memoria[dir + i] = valB >> ((3-i)*8);
   }
  break;

 //MOV a registro
 case 2:
  secReg = (A >> 4) & 0x3;
  registerMask(secReg, &corr, &mask);
  codReg = A & 0xF;
  valB = valB << (corr * 8);
  mv->registros[codReg] = (mv->registros[codReg] & (~mask)) | valB;
  printf("reg: %d\n", codReg);
  for (int i = 31; i >= 0; i--) {
   printf("%d", (mv->registros[codReg] >> i) & 1); // Imprime el bit actual
   }
   printf("\n");
   break;
 }
}

void ADD(int A, int opA, int B, int opB, TMV *mv) {
int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);
int C = valA + valB;
 printf("ADD %d+%d\n", valA, valB);
 MOV(A, opA, C, 1, mv);
 NZ(mv,C);
}

void SUB(int A, int opA, int B, int opB, TMV *mv){
int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);
int C = valA - valB;
 printf("SUB %d-%d\n", valA, valB);
 MOV(A, opA, C, 1, mv);
 NZ(mv,C);
}

void SWAP(int A, int opA, int B, int opB, TMV *mv){}

void MUL(int A, int opA, int B, int opB, TMV *mv){}

void DIV(int A, int opA, int B, int opB, TMV *mv){}

void CMP(int A, int opA, int B, int opB, TMV *mv){
int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);
 printf("CMP %d-%d=%d\n", valA, valB, valA-valB);
 NZ(mv,valA-valB);
}

void SHL(int A, int opA, int B, int opB, TMV *mv){}

void SHR(int A, int opA, int B, int opB, TMV *mv){}

void AND(int A, int opA, int B, int opB, TMV *mv){}

void OR(int A, int opA, int B, int opB, TMV *mv){}

void XOR(int A, int opA, int B, int opB, TMV *mv){}

void RND(int A, int opA, int B, int opB, TMV *mv){}

void SYS(int A, int opA, int B, int opB, TMV *mv){
int dir = ((mv->tablaSegmentos[(mv->registros[13]>>16)&0x1]>>16)&0xFFFF) + (mv->registros[13]&0xFFFF);
int cant = mv->registros[12]&0xFF;
int tam = (mv->registros[12]>>8)&0xFF;
int modo = mv->registros[10]&0xF;
int x,i,k;

 printf("sys dir:%d A:%d tam:%d cant:%d modo:%d\n",dir,A,tam,cant,modo);
 switch(A){

  case 1:
   for(i=0; i<cant; i++){
    printf("\n[%04X]: ",dir);
    Input(&x,modo);
    for(k=tam-1; k>=0; k--){
     mv->memoria[dir++] = (char) (x&(0xFF<<8*k))>>(8*k);
     }
   }
 break;

  case 2:
   for(i=0; i<cant; i++){
    x=0;
    printf("\n[%04X]: ",dir);
    for(k=tam-1; k>=0; k--){
     x |= (mv->memoria[dir++]<<(8*k));
     }
    Output(x,modo);
   }
   break;
  }
}

void Input(int *x,int modo){
char c;
 switch(modo){
  case 1:
      scanf("%d",x);
      break;
  case 2:
      scanf("%c",&c);
      *x = c;
      break;
  case 4:
      scanf("%o",x);
      break;
  case 8:
      scanf("%x",x);
 }
}

void Output(int x, int modo){
 switch(modo){
  case 1:
      printf("%d",x);
      break;
  case 2:
      printf("%c",x);
      break;
  case 4:
      printf("%o",x);
      break;
  case 8:
      printf("%x",x);
 }
}


void JMP(int A, int opA, int B, int opB, TMV *mv){
 printf("JMP a %d \n",A);
 mv->registros[5] = mv->registros[0]+A;
};

void JZ(int A, int opA, int B, int opB, TMV *mv){

if(mv->registros[8]&0x40000000){
 mv->registros[5] = mv->registros[0]+A;
 printf("JZ a %d\n",A);
}
};

void JP(int A, int opA, int B, int opB, TMV *mv){

if((!(mv->registros[8]&0x80000000)) && (!(mv->registros[8]&0x40000000))){
 mv->registros[5] = mv->registros[0]+A;
 printf("JP a %d\n",A);
}
};

void JN(int A, int opA, int B, int opB, TMV *mv){

if(mv->registros[8]&0x80000000){
 mv->registros[5] = mv->registros[0]+A;
 printf("JN a %d\n",A);
}
};

void JNZ(int A, int opA, int B, int opB, TMV *mv){

if(!(mv->registros[8]&0x40000000)){
 mv->registros[8] = mv->registros[0]+A;
 printf("JNZ a %d\n",A);
}
};

void JNP(int A, int opA, int B, int opB, TMV *mv){

if((mv->registros[8]&0x80000000) || (mv->registros[8]&0x40000000)){
 mv->registros[5] = mv->registros[0]+A;
 printf("JNP a %d\n",A);
}
};

void JNN(int A, int opA, int B, int opB, TMV *mv){

if(!(mv->registros[8]&0x80000000)){
 mv->registros[5] = mv->registros[0]+A;
 printf("JNN a %d\n",A);
}
};

void LDL(int A, int opA, int B, int opB, TMV *mv){}

void LDH(int A, int opA, int B, int opB, TMV *mv){}

void NOT(int A, int opA, int B, int opB, TMV *mv){}


