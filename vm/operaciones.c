#include <stdio.h>
#include <stdlib.h>
#include "operaciones.h"


void MOV(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){
int SegReg,dir,IP = Registros[5],corr;

printf("mov %d %d\n",opA,B);
switch(opA){
 case 0:
     dir = Direccion(Memoria,TablaSegmento,Registros,IP-4);
     for(int i=0;i<4;i++)
       Memoria[dir+i] = (char) ((B & (0xFF<<(24-8*i))) >>(24-8*i));
     break;
 case 2:
     SegRegister(&SegReg,&corr,Memoria,IP-1);
     Registros[Memoria[(IP-1)]&0xF] = Registros[Memoria[(IP-1)]&0xF] | ((B<<corr*8) & SegReg);
     printf("reg: %d\n",Memoria[(IP-1)]&0xF);
   for (int i = 31; i >= 0; i--) {
            printf("%d", (Registros[Memoria[(IP-1)]&0xF] >> i) & 1); // Imprime el bit actual
        }
        printf("\n");
     break;
 }
}

void ADD(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){
int C = A + B,SegReg,dir,IP = Registros[5],corr;

printf("add %d+%d\n",A,B);
switch(opA){
 case 0:
     dir = Direccion(Memoria,TablaSegmento,Registros,IP-4);
     for(int i=0;i<4;i++)
       Memoria[dir+i] = (char) ((C & (0xFF<<(24-8*i))) >>(24-8*i));
     break;
 case 2:
     SegRegister(&SegReg,&corr,Memoria,IP-1);
     Registros[Memoria[(IP-1)]&0xF] = Registros[Memoria[(IP-1)]&0xF] | ((C<<corr*8) & SegReg);
     printf("reg: %d\n",Memoria[(IP-1)]&0xF);
   for (int i = 31; i >= 0; i--) {
            printf("%d", (Registros[Memoria[(IP-1)]&0xF] >> i) & 1); // Imprime el bit actual
        }
        printf("\n");
     break;
 }

};

void SYS(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){
int dir = ((TablaSegmento[(Registros[13]>>16)&0xFFFF]>>16)&0xFFFF) + (Registros[13]&0xFFFF);
int cant = Registros[12]&0xFF;
int tam = ((Registros[12]&0xFF00)>>8)&0xFF;
int modo = Registros[10]&0xFF;
int x;

printf("sys dir%d A%d tam%d cant%d modo%d\n",dir,A,tam,cant,modo);
switch(A){
    //lee
 case 1:
        for(int i=0;i<cant;i++){
         printf("[%04X]: ",dir);
         E_S(&x,modo,1);
          for(int h=tam;h>0;h--){
           Memoria[dir] = (char)((x&(0xFF<<(h-1)*8))>>(h-1)*8);
           for (int i = 7; i >= 0; i--) {
            printf("%d", ((int)Memoria[dir] >> i) & 1); // Imprime el bit actual
        }
        dir++;
          }
        }
        printf("\n");
        break;
    //escribe
 case 2:
        x=0;
        for(int i=0;i<cant;i++){
         printf("[%04X]: ",dir);
          for(int h=tam;h>0;h--){
           x = x | (int)((Memoria[dir]&0xFF)<<(h-1)*8);
           dir++;
          }
          E_S(&x,modo,2);
        }
}

};

void E_S(int *x,int modo,int t){
char c;
//el t es parar saber si en sys estoy leyendo o escribiendo esta horrible la func
if(t==1){
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
}else{

 switch(modo){
  case 1:
      printf("%d",*x);
      break;
  case 2:
      printf("%c",*x);
      break;
  case 4:
      printf("%o",*x);
      break;
  case 8:
      printf("%x",*x);
 }
}

}

//te devuelve una mascara para el segmento del registro q se quiere usar
void SegRegister(int *mask,int *seg,char* Memoria,int IP){
 *seg = (((int)(Memoria[IP] & 0x30))>>4) & 0x03;

switch(*seg){
    case 0: *mask = 0xFFFFFFFF;
      break;
    case 1: *mask = 0xFF;
            *seg = 0;
      break;
    case 2: *mask = 0xFF00;
            *seg = 1;
      break;
    case 3: *mask = 0xFFFF;
            *seg = 0;
      break;
   }
}

// te devuelve la direccion de memoria que contiene el dato a usar (es un kilombo)
//basicamente se fija en los 16 bits mas significativos asi busca en la tabla de segmento de donde parto
// despues a ese origen le sumo el offset de los 16 bits menos signfiicativos
int Direccion(char* Memoria, int TablaSegmento[],int* Registros,int IP){
  return ((TablaSegmento[(Registros[(int)Memoria[IP+1]&0xF]>>16)&0xFFFF]>>16)&0xFFFF)+(Registros[(int)Memoria[IP+1]&0xF]&0xFFFF)+((((int)(Memoria[IP+2])&0xFF)<<8)|((int)Memoria[IP+3]&0xFF));
}


void SUB(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void SWAP(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void MUL(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void DIV(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void CMP(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void SHL(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void SHR(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void AND(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void OR(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void XOR(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void RND(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void JMP(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void JZ(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void JP(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void JN(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void JNZ(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void JNP(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void JNN(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void LDL(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void LDH(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};

void NOT(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]){};
