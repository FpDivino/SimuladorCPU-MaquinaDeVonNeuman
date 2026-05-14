unsigned char memoria[256];


unsigned int mbr;        // 32 bits 
unsigned short int mar;  // 16 bits 
unsigned short int pc;   // 16 bits 
unsigned short int imm;  // 16 bits 

unsigned char ir;         // 8 bits, mas usa 5 bits
unsigned char ro0;       // 8 bits, mas usa 3 bits
unsigned char ro1;      // 8 bits, mas usa 3 bits 

unsigned char e;
unsigned char l;
unsigned char g;

unsigned short int reg[8];// r0 ate r7


int executando = 1;
int erroCPU = 0;
int tamanhoInstrucao = 0;




