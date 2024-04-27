
void disassembler(TMV mv, int programSize);

void pasoDis(TMV *mv, char instruccion);

int checkParam(int argc, char *argv[], char strCmp[]);

void mostrarOp(int tipo, int valor);

void OperandDis(TMV *mv, int tipo, int *operador);

void fillExtraDis(int extraSpace);
