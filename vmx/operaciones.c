#include "operaciones.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

//modifico los bits N,Z de acuerdo al resultado de la ultima operacion logico/matematica
void NZ(TMV *mv, int resultado)
{
    mv->registros[CC] &= 0x3FFFFFFF;

    if(resultado<0)
    {
        mv->registros[CC] |= 0x80000000;
    }

    if(!resultado)
    {
        mv->registros[CC] |= 0x40000000;
    }

}

void MOV(int A, int opA, int B, int opB, TMV *mv)
{
    int secReg, mask, codReg, dir, i, cantCelda;
    int valB = operandValue(*mv, B, opB), corr;

    switch(opA)
    {
        //MOV a memoria
        case 0:
            dir = direccion(*mv, A);
            cantCelda = (~((A>>22)&0x3))&0x3;
            for (i = cantCelda; i >=0; i--)
            {
                mv->memoria[dir++] = valB >> (i*8);
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
        MOV(AC, 2, valA%valB, 1, mv); //muevo el % a AC
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


void jump(int salto, int codOp, TMV *mv, void (*operaciones[])())
{
//verifico que el salto se mantenga dentro del code segment
//para no estar escribiendo en cada jump esta verificacion la hago aca y llamo al jump que corresponda

    if(validJump(*mv,salto))
        operaciones[codOp](salto,mv);
    else
        mv->errorFlag=1; //segmento invalido
}

void JMP(int salto, TMV *mv)
{
    mv->registros[IP] = mv->registros[CS]+salto;
};

void JZ(int salto, TMV *mv)
{
    if(mv->registros[CC]&0x40000000)
    {
        mv->registros[IP] = mv->registros[CS]+salto;
    }
};

void JP(int salto, TMV *mv)
{
    if((!(mv->registros[CC]&0x80000000)) && (!(mv->registros[CC]&0x40000000)))
    {
        mv->registros[IP] = mv->registros[CS]+salto;
    }
};

void JN(int salto, TMV *mv)
{
    if(mv->registros[CC]&0x80000000)
    {
        mv->registros[IP] = mv->registros[CS]+salto;
    }

};

void JNZ(int salto, TMV *mv)
{
    if(!(mv->registros[CC]&0x40000000))
    {
        mv->registros[CC] = mv->registros[CS]+salto;
    }

};

void JNP(int salto, TMV *mv)
{
    if((mv->registros[CC]&0x80000000) || (mv->registros[8]&0x40000000))
    {
        mv->registros[IP] = mv->registros[CS]+salto;
    }

};

void JNN(int salto, TMV *mv)
{
    if(!(mv->registros[CC]&0x80000000))
    {
        mv->registros[IP] = mv->registros[CS]+salto;
    }

};

void LDL(int A, int opA, TMV *mv)
{
    int val = operandValue(*mv,A,opA);
    mv->registros[AC] = (mv->registros[AC]&0xFFFF0000) | (val&0xFFFF);
}

void LDH(int A, int opA, TMV *mv)
{
    int val = operandValue(*mv,A,opA);
    mv->registros[AC] = (mv->registros[AC]&0xFFFF) | ((val&0xFFFF)<<16);
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

// Manejo de pila

void PUSH(int B, int opB, TMV *mv)
{
    int A = 0x060000; // el push hace un MOV a la posicion de memoria q apunta SP, entonces pongo en A un tipo de operando de memoria
    // 0 indica q escriba 4 bytes, 6 q use el registro SP para calcular direccion, 0000 q no tiene offset xq escribe donde apunta SP
    int valB = operandValue(*mv, B, opB); // este va depender de q se este pusheando, si memoria,registro o inmediato

    if(opB==2 && ((B&0xF)==11))printf(" PUSH %d\n",valB);
    mv->registros[SP] -= 4;
    if(mv->registros[SP] > mv->registros[SS])
    {
       MOV(A,0,B,opB,mv);
    }else
     {
         mv->errorFlag = 6; // stack overflow
     }
}


void POP(int A, int opA, TMV *mv)
{
    int B = 0x060000; // el pop hace un MOV de lo q tiene donde apunta SP hacia registro o memoria

    printf(" POP \n");
    if((mv->registros[SP]&0xFFFF) < (mv->tablaSegmentos[(mv->registros[SS]>>16)&0xF]&0xFFFF))
    {
        MOV(A, opA, B, 0, mv);
        mv->registros[SP] +=4;
    }
    else
    {
        mv->errorFlag = 7; // stack underflow
    }
}

// este no me gusta despues te digo xq cuando hagamos ds
void CALL(int salto, int codOp, TMV *mv)
{
    PUSH(mv->registros[IP], 1, mv);

    if(validJump(*mv,salto))
      JMP(salto,mv);
    else
      mv->errorFlag=1; //segmento invalido
}


void RET(int B, int opB, TMV *mv)
{
  printf(" RET \n");
    //preparo para hacer un POP IP q es hacer un MOV IP,[SP]
  int opA = 2; // tipo de operando registro
  int A = 0x05; // y acomodo el registro al q quiero mover lo q saque con POP como SP
  POP(A, opA, mv);

}


// LLAMADAS A SISTEMA

// el usuario ingresa por consola
void SYS1(TMV *mv)
{
    int dir = ((mv->tablaSegmentos[(mv->registros[EDX]>>16)&0xF]>>16)&0xFFFF) + (mv->registros[EDX]&0xFFFF);
    int cant = mv->registros[ECX]&0xFF;
    int tam = (mv->registros[ECX]>>8)&0xFF;
    int modo = mv->registros[EAX]&0xF;
    int x,i,k;

    for(i=0; i<cant; i++)
    {
        printf("[%04X]: ",dir);
        input(&x,modo);
        for(k=tam-1; k>=0; k--)
        {
            mv->memoria[dir++] = (x&(0xFF<<8*k))>>(8*k);
        }
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


// muestro por consola
void SYS2(TMV *mv)
{
    int dir = ((mv->tablaSegmentos[(mv->registros[EDX]>>16)&0xF]>>16)&0xFFFF) + (mv->registros[EDX]&0xFFFF);
    int cant = mv->registros[ECX]&0xFF;
    int tam = (mv->registros[ECX]>>8)&0xFF;
    int modo = mv->registros[EAX]&0xF;
    int x,i,k;

    for(i=0; i<cant; i++)
    {
        x=0;
        printf("[%04X]: ",dir);
        for(k=tam-1; k>=0; k--)
        {
            x |= (mv->memoria[dir++]<<(8*k))&(0xFF<<8*k);
        }
        output(x,modo,tam-1);
    }
}

void output(int x, int modo,int tam)
{
   if ((modo & 0x08) == 0x08)
        printf("%%%X ", x);
    if ((modo & 0x04) == 0x04)
        printf("@%o ", x);
    if ((modo & 0x02) == 0x02) {
        printf("'");
        for(int i=tam;i>=0;i--)
            printf("%c", isprint(x>>8*i)?x>>8*i:'.');
        putchar(' ');
    }
    if ((modo & 0x01) == 0x01)
        printf("#%d", x);
    putchar('\n');
}


// el usuario ingresa un string
void SYS3(TMV *mv)
{
    char buffer[1000] = {0};
    int dir = ((mv->tablaSegmentos[(mv->registros[EDX]>>16)&0xF]>>16)&0xFFFF) + (mv->registros[EDX]&0xFFFF);
    int cantCaracteres = mv->registros[ECX] & 0xFF;

    printf("[%04X]: ",dir);
    fgets(buffer, sizeof(buffer), stdin);

    if(cantCaracteres == -1)
        strcpy(mv->memoria+dir,buffer);
    else
        strncpy(mv->memoria+dir,buffer,cantCaracteres);
}


// muestro por consola un string
void SYS4(TMV *mv)
{
    int dir = ((mv->tablaSegmentos[(mv->registros[EDX]>>16)&0xF]>>16)&0xFFFF) + (mv->registros[EDX]&0xFFFF);
    printf("[%04X]: ",dir);

    while (*(mv->memoria + dir) != '\0') {
      printf("%c", isprint(*(mv->memoria + dir))?*(mv->memoria + dir):'.');
      dir++;
    }
    putchar('\n');
}

// limpio la consola (no se si es asi en C)
void SYS7(TMV *mv)
{
    system("cls");
}



// DEBUGGING

void SYSF(TMV *mv)
{
    mv->modo = DEBUG;
}

void pasoDebug(TMV *mv, char* filename)
{
    if (filename) {
        char input;

        generaImagen(filename,mv);

        do{
            input = getchar();
        }while(input != 'g' && input!='q' && input !='\n');

        if(input == 'g')
            mv->modo = !DEBUG;

        if(input == 'q')
            mv->registros[IP] |= 0x8000;
    } else {
        mv->modo = !DEBUG;
    }
}

void generaImagen(char* filename, TMV mv)
{
    FILE* arch;
    char id[] = "VMI24";
    char version = '1' - '0';
    int i;

    arch = fopen(filename,"wb");

    fwrite(id, sizeof(char), 5, arch);
    fwrite(&version, sizeof(char), 1, arch);
    fwrite(&mv.memorySize, sizeof(char), 2, arch);
    fwrite(mv.registros, sizeof(char), 64, arch);
    fwrite(mv.tablaSegmentos, sizeof(char), 32, arch);
    fwrite(mv.memoria, sizeof(char), mv.memorySize, arch);

    fclose(arch);
}
