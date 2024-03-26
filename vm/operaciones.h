
void MOV(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void ADD(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void SUB(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void SWAP(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void MUL(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void DIV(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void CMP(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void SHL(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void SHR(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void AND(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void OR(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void XOR(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void RND(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void SYS(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void JMP(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void JZ(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void JP(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void JN(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void JNZ(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void JNP(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void JNN(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void LDL(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void LDH(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void NOT(int A, int opA, int B, char Memoria[],int Registros[],int TablaSegmento[]);

void E_S(int *x,int modo,int t);

void SegRegister(int *,int *,char* ,int);

int Direccion(char* Memoria, int TablaSegmento[],int* Registros,int IP);
