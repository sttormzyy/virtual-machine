#include "operaciones.h"
#include <stdlib.h>
#include <time.h>

//modifico los bits N,Z de acuerdo al resultado de la ultima operacion logico/matematica
void NZ(TMV *mv, int resultado)
{
    mv->registros[8] &= 0x3FFFFFFF;

    if(resultado<0)
    {
        mv->registros[8] |= 0x80000000;
    }

    if(!resultado)
    {
    mv->registros[8] |= 0x40000000;
    }

}

void MOV(int A, int opA, int B, int opB, TMV *mv)
{
    int secReg, mask, codReg, dir, i;
    int valB = operandValue(*mv, B, opB), corr;

    switch(opA)
    {
        //MOV a memoria
        case 0:
            dir = direccion(*mv, A);
            for (i = 0; i < 4; ++i)
            {
                mv->memoria[dir + i] = valB >> ((3-i)*8);
            }
        break;

        //MOV a registro
        case 2:
            secReg = (A >> 4) & 0x3;
            registerMask(secReg, &corr, &mask);
            codReg = A & 0xF;
            valB = valB << (corr * 8);
            mv->registros[codReg] = (mv->registros[codReg] & (~mask)) | (valB & mask);
        break;
    }
}

void ADD(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    MOV(A, opA, valA+valB, 1, mv);
    NZ(mv,valA+valB);
}

void SUB(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    MOV(A, opA, valA-valB, 1, mv);
    NZ(mv,valA-valB);
}

void SWAP(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    MOV(A,opA,valB,1,mv);
    MOV(B,opB,valA,1,mv);
}

void MUL(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    MOV(A, opA, valA*valB, 1, mv);
    NZ(mv,valA*valB);
}

void DIV(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    if(valB)
    {
        MOV(A, opA, valA/valB, 1, mv);
        MOV(0b00001001, 2, valA%valB, 1, mv); //muevo el % a AC
        NZ(mv,valA/valB);
    }else
    {
         mv->errorFlag = 3; //division por 0
    }
}

void CMP(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    NZ(mv,valA-valB);
}

void SHL(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    MOV(A, opA, valA<<valB, 1, mv);
    NZ(mv, valA<<valB);
}

void SHR(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    MOV(A, opA, valA>>valB, 1, mv);
    NZ(mv, valA>>valB);
}

void AND(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    MOV(A, opA, valA&valB, 1, mv);
    NZ(mv,valA&valB);
}

void OR(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    MOV(A, opA, valA|valB, 1, mv);
    NZ(mv,valA|valB);
}

void XOR(int A, int opA, int B, int opB, TMV *mv)
{
    int valA = operandValue(*mv, A, opA), valB = operandValue(*mv, B, opB);

    MOV(A, opA, valA^valB, 1, mv);
    NZ(mv,valA^valB);
}

void SYS(int A, int opA, TMV *mv)
{
    int dir = ((mv->tablaSegmentos[(mv->registros[13]>>16)&0x1]>>16)&0xFFFF) + (mv->registros[13]&0xFFFF);
    int cant = mv->registros[12]&0xFF;
    int tam = (mv->registros[12]>>8)&0xFF;
    int modo = mv->registros[10]&0xF;
    int x,i,k;

    switch(A)
    {
        //read
        case 1:
            for(i=0; i<cant; i++)
            {
                printf("[%04X]: ",dir);
                input(&x,modo);
                for(k=tam-1; k>=0; k--)
                {
                    mv->memoria[dir++] = (x&(0xFF<<8*k))>>(8*k);
                }
            }
        break;
        //write
        case 2:
            for(i=0; i<cant; i++)
            {
                x=0;
                printf("[%04X]: ",dir);
                for(k=tam-1; k>=0; k--)
                {
                    x |= (mv->memoria[dir++]<<(8*k))&(0xFF<<8*k);
                }
                output(x,modo);
           }
        break;
  }
}

void input(int *x,int modo)
{
    char c;

    switch(modo)
    {
        case 1:
            scanf("%d",x);
        break;
        case 2:
            scanf("%c",&c);
            *x = c&0xFF;
            while (getchar() != '\n');
        break;
        case 4:
            scanf("%o",x);
        break;
        case 8:
            scanf("%x",x);
    }
}

void output(int x, int modo)
{
    switch(modo)
    {
        case 1:
            printf("#%d\n",x);
        break;
        case 2:
            printf("%c\n",x);
        break;
        case 4:
            printf("@%o\n",x);
        break;
        case 8:
            printf("0x%X\n",x);
   }
}

void jump(int salto, int codOp, TMV *mv, void (*operaciones[])())
{
//verifico que el salto se mantenga dentro del code segment
//para no estar escribiendo en cada jump esta verificacion la hago aca y llamo al jump que corresponda
    if(segmentoCheck(*mv,salto,2))
        operaciones[codOp](salto,mv);
    else
        mv->errorFlag=1; //segmento invalido
}

void JMP(int salto, TMV *mv)
{
    mv->registros[5] = mv->registros[0]+salto;
};

void JZ(int salto, TMV *mv)
{
    if(mv->registros[8]&0x40000000)
    {
        mv->registros[5] = mv->registros[0]+salto;
    }
};

void JP(int salto, TMV *mv)
{
    if((!(mv->registros[8]&0x80000000)) && (!(mv->registros[8]&0x40000000)))
    {
        mv->registros[5] = mv->registros[0]+salto;

    }
};

void JN(int salto, TMV *mv)
{
    if(mv->registros[8]&0x80000000)
    {
        mv->registros[5] = mv->registros[0]+salto;
    }
};

void JNZ(int salto, TMV *mv)
{
    if(!(mv->registros[8]&0x40000000))
    {
        mv->registros[8] = mv->registros[0]+salto;

    }
};

void JNP(int salto, TMV *mv)
{
    if((mv->registros[8]&0x80000000) || (mv->registros[8]&0x40000000))
    {
        mv->registros[5] = mv->registros[0]+salto;
    }
};

void JNN(int salto, TMV *mv)
{
    if(!(mv->registros[8]&0x80000000))
    {
        mv->registros[5] = mv->registros[0]+salto;
    }
};

void LDL(int A, int opA, TMV *mv)
{
    int val = operandValue(*mv,A,opA);
    mv->registros[9] = (mv->registros[9]&0xFFFF0000) | (val&0xFFFF);
}

void LDH(int A, int opA, TMV *mv)
{
    int val = operandValue(*mv,A,opA);
    mv->registros[9] = (mv->registros[9]&0xFFFF) | ((val&0xFFFF)<<16);
}

void NOT(int A, int opA, TMV *mv)
{
    int val = operandValue(*mv, A, opA);

    MOV(A, opA, ~val, 1, mv);
    NZ(mv,~val);
}

void RND(int A, int opA, int B, int opB, TMV *mv)
{
    int valB = operandValue(*mv, B, opB);

    srand(time(NULL));
    int C = rand()%(valB+1); //al hacer %(valB+1) hago q devuelva num entre 0 y valB
    MOV(A,opA,C,1,mv);
}

