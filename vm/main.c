#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operaciones.h"


void Header(int *ProgramSize,int *valido,char * filename);
void CargaCodigo(char Memoria[],char* filename,int);
void Inicializacion(int TablaSegmento[],int Registros[],int ProgramSize);
void ProcesaInstruccion(char* Memoria, int* Registros, int TablaSegmento[],void (*Operaciones[23])());
void valorOperando(int tipo,int* op,char* Memoria, int* Registros, int TablaSegmento[]);


int main(int argc, char *argv[])
{
    int ProgramSize = 0;
    void (*Operaciones[24])()={MOV,ADD,SUB,SWAP,MUL,DIV,CMP,SHL,SHR,AND,OR,XOR,
                               RND,SYS,JMP,JZ,JP,JN,JNZ,JNP,JNN,LDL,LDH,NOT};
    char Memoria[16384];
    int Registros[16];
    int TablaSegmento[2],valido;


    // verifica  q el archivo sea del tipo correcto y almacena el tamanio del programa en bytes
    Header(&ProgramSize,&valido,argv[1]);

    if(ProgramSize && valido){

     // cargo el codigo en la memoria principal
     CargaCodigo(Memoria,argv[1],ProgramSize);

     // inicializo registros CS,DS,IP y Tabla de Segmento
     Inicializacion(TablaSegmento,Registros,ProgramSize);

     // Procesa programa hasta encontrar un STOP
     while((Memoria[Registros[5]]&0xFF)!=0xFF){
      ProcesaInstruccion(Memoria,Registros,TablaSegmento,Operaciones);
      }
    }

    return 0;
}

void Header(int *ProgramSize,int *valido,char * filename){
FILE *arch = fopen(filename,"rb");
char tipoArch[6],x,y;

fread(tipoArch,sizeof(char),5,arch);

if(!strcmp(tipoArch,"VMX24")){
 fread(&x,sizeof(char),1,arch);
 *valido = 1;
 fread(&x,sizeof(char),1,arch);
 fread(&y,sizeof(char),1,arch);
 *ProgramSize = (int)x<<8 | (int)y;
  printf("Size %d\n",*ProgramSize);
}else{
    *valido = 0;
 }
}

void CargaCodigo(char Memoria[],char* filename,int ProgramSize){
FILE *arch = fopen(filename,"rb");

//me salteo header
fseek(arch,8,0);

fread(Memoria,sizeof(char),ProgramSize,arch);


//para mostrar por pantalla el codigo en memoria
for(int h=0;h<ProgramSize;h++){
        printf("%d:",h);
    for (int i = 7; i >= 0; i--) {
            printf("%d", (Memoria[h] >> i) & 1); // Imprime el bit actual
        }
printf("\n");
}

}

void Inicializacion(int TablaSegmento[],int Registros[],int ProgramSize){
    Registros[0] = Registros[1] = Registros[5] = 0;

    Registros[1] = Registros[1] | (1<<16);

    TablaSegmento[0] = TablaSegmento[1] = 0;
    TablaSegmento[0] = ProgramSize;
    TablaSegmento[1] =  (ProgramSize<<16) | (16384-ProgramSize);
}

void ProcesaInstruccion(char* Memoria, int* Registros, int TablaSegmento[],void (*Operaciones[23])()){
int A,B,opA,opB,codOp,IP=Registros[5];

//determino tipos de operandos y codigo de operacion
opB = (((int)Memoria[IP] & 0xC0)>>6) & 0x03;
opA = ((int)Memoria[IP] & 0x30)>>4;
codOp = (int)Memoria[IP] & 0x1F;


//obtengo valor operandos y acomodo el IP a siguiente instruccion
//si el operando A es ninguno no busco segundo valor de operando
valorOperando(opB,&B,Memoria,Registros,TablaSegmento);
if(opA!=3)
valorOperando(opA,&A,Memoria,Registros,TablaSegmento);

Registros[5]++;

//segun el codigo de operacion, llamo la q corresponda (distingo entre ops de 1 y 2 operandos)
if((codOp & 0x10)!=0)
 Operaciones[codOp-3](B,opB,A,Memoria,Registros,TablaSegmento);
else
  Operaciones[codOp](A,opA,B,Memoria,Registros,TablaSegmento);
}

void valorOperando(int tipo,int* op,char* Memoria, int* Registros, int TablaSegmento[]){
int segReg,dir,IP=Registros[5],corr;

switch(tipo){

 //memoria
  case 0:
      *op=0;
      dir = Direccion(Memoria,TablaSegmento,Registros,IP);
      printf("dir %d\n",dir);
      for(int i=4;i>0;i--){
        *op = *op | ((int)Memoria[dir]<<8*(i-1));
        dir++;
      }
      Registros[5]+=3;
      break;

  //inmediato
  case 1:
       *op = ((int)Memoria[IP+1])<<8 | ((int)Memoria[IP+2]&0xFF);
       Registros[5]+=2;
       break;

  //Registro
  case 2:
       SegRegister(&segReg,&corr,Memoria,IP+1);
       //printf("%d\n",(int)Memoria[IP+1]&0xF);
       *op = ((Registros[(int)Memoria[IP+1]&0xF] & segReg)>>8*corr);
       //trato como caso especial si quiero usar la parte ff00 o high
       if(corr){
        *op = *op & 0xFF;
       }
       Registros[5]+=1;
       break;
    }

}



