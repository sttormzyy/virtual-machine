#include <stdlib.h>
#include <string.h>
#include "operaciones.h"
#include "disassembler.h"


void procesaInstruccion(TMV *mv, void (*operaciones[])(), void (*systemCall[])());

void reportStatus(int estado);

int finPrograma(TMV mv);

int main(int argc, char *argv[]){
    TMV mv;
    char* imageFilename;
    int segmentoSizes[6], i, programSize;
    srand(time(NULL)); //para la funcion rnd

    void (*operaciones[])() = {MOV, ADD, SUB, SWAP, MUL, DIV, CMP, SHL, SHR, AND, OR, XOR,
                                 RND, NULL, NULL, NULL, NULL, JMP, JZ, JP, JN, JNZ, JNP, JNN,
                                 LDL, LDH, NOT, PUSH, POP, CALL, RET};

    void (*systemCall[])() = {SYS1, SYS2, SYS3, SYS4, NULL, NULL, SYS7, NULL, NULL, NULL, NULL, NULL, NULL, NULL, SYSF};




    i = checkParam( argc, argv, "m=");
    mv.memorySize = (i!=0) ? atoi(argv[i]+2)*0x400 : MEMORIA_SIZE;

    i = checkParam( argc, argv, ".vmi");
    imageFilename = (i!=0)? argv[i] : NULL;

    inicializacion(segmentoSizes, argv[1], &mv);

    //chequeo si pidieron disassembler
    if (checkParam( argc, argv, "-d")) {
        programSize = (mv.tablaSegmentos[mv.registros[CS]>>16] >> 16) + (mv.tablaSegmentos[mv.registros[CS]>>16]&0xFFFF);
        disassembler(mv, programSize);
    }

    //procesa programa hasta encontrar un STOP o que ocurra un error (segmento invalido,div por 0,etc)
    while (!finPrograma(mv))
    {
        procesaInstruccion(&mv, operaciones, systemCall);

        if(mv.modo == DEBUG)
            pasoDebug(&mv, imageFilename);

    }

    reportStatus(mv.errorFlag);
    return 0;
}

int finPrograma(TMV mv)
{
    return mv.errorFlag || ((instruccionActual(mv)&0xFF)==0xFF) || !validIP(mv);
}

void procesaInstruccion(TMV *mv, void (*operaciones[])(), void (*systemCall[])()){
    int A, B, opA, opB, codOp, vA, vB, i;
    char instruccion = instruccionActual(*mv);
    char* aux;
    codOp = instruccion & 0x1F;

    if( instruccionValida(codOp))
    {
        //determino tipos de operandos
        opB = ((instruccion & 0xC0) >> 6) & 0x03;
        opA = (instruccion & 0x30) >> 4;

        mv->registros[IP]++;

        //obtengo operandos
        readOperand(mv, opB, &B);
        readOperand(mv, opA, &A);

        //si el tipo de operando es de memoria, chequeo validez de la direccion
        vA = (opA == 0)? validDirection(*mv,A) : 1;
        vB = (opB == 0)? validDirection(*mv,B) : 1;

        if(vA && vB)
        {
            //segun el codigo de operacion, llamo la q corresponda
            if(codOp & 0x10)
            {
                if(codOp == 0x10)
                    systemCall[B-1](mv); //llamadas a sistema
                else
                {
                    if((codOp>=0x11 && codOp<=0x17))
                        jump(B, codOp, mv, operaciones); //jumps
                    else
                        operaciones[codOp](B, opB, mv); //operaciones de 1 operando
                }
            }else
                operaciones[codOp](A, opA, B, opB, mv); //operaciones de 2 operandos

        }else
            mv->errorFlag = 1; //segmento invalido
    }else
        mv->errorFlag = 2; //instruccion invalida
}

void reportStatus(int estado)
{
    switch(estado)
    {
        case 0: printf("\n\nSUCCESSFUL EXECUTION :)");
        break;
        case 1: printf("\n\nINVALID SEGMENT");
        break;
        case 2: printf("\n\nINVALID INSTRUCTION");
        break;
        case 3: printf("\n\nDIVISION BY ZERO");
        break;
        case 4: printf("\n\nINVALID ARCHIVE TYPE");
        break;
        case 5: printf("\n\nINSUFFICIENT MEMORY");
        break;
        case 6: printf("\n\nSTACK OVERFLOW");
        break;
        case 7: printf("\n\nSTACK UNDERFLOW");
        break;
        default: printf("\n\nUNHANDLED ERROR");
        break;
   }
}
