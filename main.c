#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mvn.h"

unsigned char lerByteMemoria(unsigned short int endereco) {
    mar = endereco & 0x00FF;
    mbr = memoria[mar] & 0xFF;

    return (unsigned char)(mbr & 0xFF);
}


//Vanessa
void escreverByteMemoria(unsigned short int endereco, unsigned char dado) {
    mar = endereco & 0x00FF;
    mbr = dado & 0xFF;
    memoria[mar] = mbr;
}

void inicializarCPU(void) {
    pc = 0;
    int i;

    for (i = 0; i < 256; i++) {
        memoria[i] = 0xFF;
    }

    for (i = 0; i < 8; i++) {
        reg[i] = 0xFFFF;
    }

    mbr = 0xFFFFFFFF;
    mar = 0xFFFF;
    pc = 0x0000;
    imm = 0xFFFF;

    ir = 0xFF;
    ro0 = 0xFF;
    ro1 = 0xFF;

    e = 0xFF;
    l = 0xFF;
    g = 0xFF;

    executando = 1;
    erroCPU = 0;
    tamanhoInstrucao = 0;
}

// prototipo pq carregarMemoriaArquivo usa codificarInstrucao antes dela ser definida
void codificarInstrucao(const char *mnem, unsigned char endereco);

// le o arquivo e carrega na memoria
// formato: endereco;i ou d;instrucao ou dado
// ex: 0;i;ld r0, 1e   ou   90;d;20
void carregarMemoriaArquivo(const char *nomeArquivo) {
    FILE *arq = fopen(nomeArquivo, "r");
    if (arq == NULL) {
        printf("erro: nao abriu o arquivo %s\n", nomeArquivo);
        return;
    }

    char linha[128];

    while (fgets(linha, sizeof(linha), arq)) {
        linha[strcspn(linha, "\n")] = '\0'; // tira o \n

        if (strlen(linha) == 0) continue; // linha vazia

        char *parteEndereco = strtok(linha, ";");
        char *parteTipo     = strtok(NULL, ";");
        char *parteConteudo = strtok(NULL, ";");

        if (parteEndereco == NULL || parteTipo == NULL || parteConteudo == NULL)
            continue;

        unsigned int endereco = (unsigned int)strtol(parteEndereco, NULL, 16);

        // hlt escreve o byte mas continua lendo o arquivo pq pode ter dados depois
        if (strcmp(parteConteudo, "hlt") == 0) {
            memoria[endereco & 0xFF] = 0x00;
            continue;
        }

        if (parteTipo[0] == 'd') {
            // dado — dois bytes, big endian
            unsigned int valor = (unsigned int)strtol(parteConteudo, NULL, 16);
            memoria[(endereco)     & 0xFF] = (valor >> 8) & 0xFF;
            memoria[(endereco + 1) & 0xFF] = valor & 0xFF;

        } else if (parteTipo[0] == 'i') {
            // instrucao — monta os bytes
            codificarInstrucao(parteConteudo, endereco & 0xFF);
        }
    }

    fclose(arq);
}

// recebe o mnemônico e escreve os bytes na memoria
// ex: "ld r0, 1e"  ->  A8 00 1E
void codificarInstrucao(const char *mnem, unsigned char endereco) {
    char buf[64];
    strncpy(buf, mnem, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *op = strtok(buf, " ,\t"); // pega o opcode
    if (op == NULL) return;

    // 1 byte
    if (strcmp(op, "hlt") == 0) {
        memoria[endereco] = 0x00; // opcode 00000
        return;
    }
    if (strcmp(op, "nop") == 0) {
        memoria[endereco] = 0x08; // opcode 00001
        return;
    }

    // not rX — 1 byte com reg0
    if (strcmp(op, "not") == 0) {
        char *sReg0 = strtok(NULL, " ,\t");
        unsigned char r0 = (sReg0 != NULL) ? (unsigned char)atoi(sReg0 + 1) : 0;
        memoria[endereco] = (0b01101 << 3) | (r0 & 0x07);
        return;
    }

    // Intruções de 2 bytes: op rX, rY
    struct { const char *nome; unsigned char opcode; } ops2[] = {
        {"ldr", 0b00010}, {"str", 0b00011},                 // 00010 = ldr    00011 = str
        {"add", 0b00100}, {"sub", 0b00101},                 // 00100 = add    00101 = sub
        {"mul", 0b00110}, {"div", 0b00111},                 // 00110 = mul    00111 = div
        {"cmp", 0b01000}, {"movr",0b01001},                 // 01000 = cmp    01001 = movr
        {"and", 0b01010}, {"or",  0b01011}, {"xor", 0b01100}, // 01010 = and    01011 = or    01100 = xor
        {NULL, 0}
    };

    int i;
    for (i = 0; ops2[i].nome != NULL; i++) {
        if (strcmp(op, ops2[i].nome) == 0) {
            char *sReg0 = strtok(NULL, " ,\t");
            char *sReg1 = strtok(NULL, " ,\t");
            unsigned char r0 = (sReg0 != NULL) ? (unsigned char)atoi(sReg0 + 1) : 0;
            unsigned char r1 = (sReg1 != NULL) ? (unsigned char)atoi(sReg1 + 1) : 0;
            memoria[endereco]     = (ops2[i].opcode << 3) | (r0 & 0x07);
            memoria[endereco + 1] = (r1 & 0x07) << 5;
            return;
        }
    }

    /*
        Instrucoes de 3 bytes:
        jumps — sem reg0, so endereco
        ld, st, movi, addi, subi, muli, divi, lsh, rsh — reg0 + endereco ou imediato
    */

    // jumps: op Z
    struct { const char *nome; unsigned char opcode; } jumps[] = {
        {"je",  0b01110}, {"jne", 0b01111},   // 01110 = je     01111 = jne
        {"jl",  0b10000}, {"jle", 0b10001},   // 10000 = jl     10001 = jle
        {"jg",  0b10010}, {"jge", 0b10011},   // 10010 = jg     10011 = jge
        {"jmp", 0b10100},                     // 10100 = jmp
        {NULL, 0}
    };

    for (i = 0; jumps[i].nome != NULL; i++) {
        if (strcmp(op, jumps[i].nome) == 0) {
            char *sImm = strtok(NULL, " ,\t");
            unsigned short int imediato = (sImm != NULL) ?
                (unsigned short int)strtol(sImm, NULL, 16) : 0;
            memoria[endereco]     = (jumps[i].opcode << 3) & 0xF8;
            memoria[endereco + 1] = (imediato >> 8) & 0xFF;
            memoria[endereco + 2] = imediato & 0xFF;
            return;
        }
    }

    // op rX, Z ou op rX, IMM
    struct { const char *nome; unsigned char opcode; } ops3[] = {
        {"ld",   0b10101}, {"st",   0b10110},   // 10101 = ld     10110 = st
        {"movi", 0b10111}, {"addi", 0b11000},   // 10111 = movi   11000 = addi
        {"subi", 0b11001}, {"muli", 0b11010},   // 11001 = subi   11010 = muli
        {"divi", 0b11011}, {"lsh",  0b11100},   // 11011 = divi   11100 = lsh
        {"rsh",  0b11101},                      // 11101 = rsh
        {NULL, 0}
    };

    for (i = 0; ops3[i].nome != NULL; i++) {
        if (strcmp(op, ops3[i].nome) == 0) {
            char *sReg0 = strtok(NULL, " ,\t");
            char *sImm  = strtok(NULL, " ,\t");
            unsigned char r0 = (sReg0 != NULL) ? (unsigned char)atoi(sReg0 + 1) : 0;
            unsigned short int imediato = (sImm != NULL) ?
                (unsigned short int)strtol(sImm, NULL, 16) : 0;
            memoria[endereco]     = (ops3[i].opcode << 3) | (r0 & 0x07);
            memoria[endereco + 1] = (imediato >> 8) & 0xFF;
            memoria[endereco + 2] = imediato & 0xFF;
            return;
        }
    }

    printf("instrucao nao reconhecida: %s\n", mnem);
}


void busca(void) {
    unsigned char primeiroByte = 0;
    unsigned char segundoByte = 0;
    unsigned char terceiroByte = 0;

    mbr = 0;

    primeiroByte = lerByteMemoria(pc);

    ir = primeiroByte >> 3;    // O opcode esta nos bits 7..3 do primeiro byte. O primeiroByte tem 8 bits, deslocar 3 bits para a direita
                               // deixa apenas o opcode.

    if (ir == 0b00000 || ir == 0b00001 || ir == 0b01101) {    // Instruções de 1 byte
        tamanhoInstrucao = 1;                                 // 00000 = halt
        mbr = primeiroByte;                                   // 00001 = nop
    }                                                         // 01101 = not

    else if (ir >= 0b00010 && ir <= 0b01100) {                // Intruções de 2 bytes
        tamanhoInstrucao = 2;                                 // 00010 = ldr                00111 = div
                                                              // 00011 = str                01000 = cmp
        segundoByte = lerByteMemoria(pc + 1);                 // 00100 = add                01001 = movr
                                                              // 00101 = sub                01010 = and
        mbr = ((unsigned int)primeiroByte << 8) |             // 00110 = mul                01100 = xor
              ((unsigned int)segundoByte);
    }

    /*
        Instrucoes de 3 bytes:
        01110 ate 11101
        saltos, ld, st, movi, addi, subi, muli, divi, lsh, rsh
    */
    else if (ir >= 0b01110 && ir <= 0b11101) {                // Instruções de 3 bytes
        tamanhoInstrucao = 3;                                 // 01110 = je                10110 = st
                                                              // 01111 = jne               10111 = movi
        segundoByte = lerByteMemoria(pc + 1);                 // 10000 = jl                11000 = addi
        terceiroByte = lerByteMemoria(pc + 2);                // 10001 = jle               11001 = subi
                                                              // 10010 = jg                11010 = muli
        mbr = ((unsigned int)primeiroByte << 16) |            // 10011 = jge               11011 = divi
              ((unsigned int)segundoByte << 8)  |             // 10100 = jmp               11100 = lsh
              ((unsigned int)terceiroByte);                   // 10101 = ld                11101 = rsh
    }

    else {
        printf("\nERRO: opcode invalido encontrado em PC = 0x%04X\n", pc);
        printf("Byte encontrado: 0x%02X\n", primeiroByte);

        erroCPU = 1;
        executando = 0;
        tamanhoInstrucao = 0;
        mbr = primeiroByte;
    }
}

/*
    DECODIFICAO:
    1 byte:
        bits 7..3 = opcode
        bits 2..0 = reg0 ou zero

    2 bytes:
        primeiro byte:
            bits 7..3 = opcode
            bits 2..0 = reg0

        segundo byte:
            bits 7..5 = reg1
            bits 4..0 = zero

    3 bytes:
        primeiro byte:
            bits 7..3 = opcode
            bits 2..0 = reg0 ou zero

        segundo e terceiro bytes:
            endereco ou imediato de 16 bits
*/

void decodifica(void) {
    unsigned char primeiroByte = 0;
    unsigned char segundoByte = 0;
    unsigned char terceiroByte = 0;

    ro0 = 0;
    ro1 = 0;
    imm = 0;

    if (tamanhoInstrucao == 1) {
        primeiroByte = mbr & 0xFF;

        ir = primeiroByte >> 3;
        ro0 = primeiroByte & 0b00000111;
    }

    else if (tamanhoInstrucao == 2) {
        primeiroByte = (mbr >> 8) & 0xFF;
        segundoByte = mbr & 0xFF;

        ir = primeiroByte >> 3;
        ro0 = primeiroByte & 0b00000111;
        ro1 = (segundoByte >> 5) & 0b00000111;
    }

    else if (tamanhoInstrucao == 3) {
        primeiroByte = (mbr >> 16) & 0xFF;
        segundoByte = (mbr >> 8) & 0xFF;
        terceiroByte = mbr & 0xFF;

        ir = primeiroByte >> 3;
        ro0 = primeiroByte & 0b00000111;

        imm = ((unsigned short int)segundoByte << 8) |
              ((unsigned short int)terceiroByte);
    }

    else {
        printf("\nERRO: tamanho de instrucao invalido: %d\n", tamanhoInstrucao);

        erroCPU = 1;
        executando = 0;
    }
}

// mostra registradores e memoria no formato pedido
void exibirEstado(void) {
    int i, j;

    printf("\nCPU:\n");
    printf("R0: %04X  R1: %04X  R2: %04X  R3: %04X\n",
           reg[0], reg[1], reg[2], reg[3]);
    printf("R4: %04X  R5: %04X  R6: %04X  R7: %04X\n",
           reg[4], reg[5], reg[6], reg[7]);
    printf("MBR: %08X  MAR: %04X  IMM: %04X  PC: %04X\n",
           mbr, mar, imm, pc);
    printf("IR: %02X  RO0: %X  RO1: %X\n", ir, ro0, ro1);
    printf("E: %X  L: %X  G: %X\n", e, l, g);

    printf("\nMemoria:\n");
    printf("    ");
    for (i = 0; i < 16; i++) {
        printf("%02X ", i);
    }
    printf("\n");

    for (i = 0; i < 16; i++) {
        printf("%02X  ", i * 16);
        for (j = 0; j < 16; j++) {
            printf("%02X ", memoria[i * 16 + j]);
        }
        printf("\n");
    }
}

//Vanessa
void executa(void) {
    unsigned char byteAlto;
    unsigned char byteBaixo;

    switch (ir) {
        case 0b00000: //hlt
            executando = 0;
            break;

        case 0b00001://nop
            break;

        case 0b00010: { //ldr
            unsigned char byteAlto  = lerByteMemoria(reg[ro1]);
            unsigned char byteBaixo = lerByteMemoria(reg[ro1] + 1);
            reg[ro0] = ((unsigned short int)byteAlto << 8) | byteBaixo;
            break;
        }

        case 0b00011: // str rX, rY
            byteAlto = reg[ro0] >> 8;
            byteBaixo = reg[ro0] & 0xFF;
            escreverByteMemoria(reg[ro1], byteAlto);
            escreverByteMemoria(reg[ro1] + 1, byteBaixo);
            break;

        case 0b00100: { // add rX, rY
            reg[ro0] = reg[ro0] + reg[ro1];
            break;
        }

        case 0b00101: {//sub
            reg[ro0] = reg[ro0] - reg[ro1];
            break;
        }

        case 0b00110: { //mull
            reg[ro0] = reg[ro0] * reg[ro1];
            break;
        }

        case 0b00111: { //div
            if (reg[ro1] == 0) {
                erroCPU = 1;
                executando = 0;
             break;
            }
            reg[ro0] = reg[ro0] / reg[ro1];
            break;
        }

        case 0b01000: { //cmp
            if (reg[ro1] == reg[ro0]) {
                e = 1;
            } else {
                e = 0;
            }

            if (reg[ro0] < reg[ro1]) {
                l = 1;
            } else {
                l = 0;
            }

            if (reg[ro0] > reg[ro1]) {
                g = 1;
            } else {
                g = 0;
            }

            break;
        }

        case 0b01001: { //movr
            reg[ro0] = reg[ro1];
        break;
        }

        case 0b01010: { //and
            reg[ro0] = reg[ro0] & reg[ro1];
            break;
        }

        case 0b01011: { //or
            reg[ro0] = reg[ro0] | reg[ro1];
        break;
        }

        case 0b01100: { //xor
            reg[ro0] = reg[ro0] ^ reg[ro1];
            break;
        }

         case 0b01101: { //not rx
            reg[ro0] = ~reg[ro0];
         break;
        }

         case 0b01110: {
            // je z
            if (e == 1) {
                tamanhoInstrucao = 0 ;
                pc = imm & 0x00FF;
                break;
            }

            break;
        }

        case 0b01111: {
            //jne z
            if (e == 0) {
                tamanhoInstrucao = 0;
                pc = imm & 0x00FF;
                break;
            }

            break;
        }

        case 0b10000: { // jl z
            if (l == 1) {
                tamanhoInstrucao = 0;
                pc = imm & 0x00FF;
            }
        break;
        }

        case 0b10001: { // jle z
            if (l == 1 || e == 1) {
                tamanhoInstrucao = 0;
                pc = imm & 0x00FF;
            }
        break;
        }

        case 0b10010: { // jg z
            if (g == 1) {
                tamanhoInstrucao = 0;
                pc = imm & 0x00FF;
            }
            break;
        }

        case 0b10011: { // jge z
            if (g == 1  || e == 1) {
                tamanhoInstrucao = 0;
                pc = imm & 0x00FF;
            }
        break;
        }

         case 0b10100: { //jmp
            tamanhoInstrucao = 0;
            pc = imm & 0x00FF;
            break;
        }

         case 0b10101: { //ld
            unsigned char byteAlto = lerByteMemoria(imm);
            unsigned char byteBaixo = lerByteMemoria(imm + 1);
            reg[ro0] = ((unsigned short int) byteAlto << 8) | byteBaixo ;
            break;
         }

         case 0b10110: { //st
            escreverByteMemoria(imm,reg[ro0]>>8);
            escreverByteMemoria(imm+1,reg[ro0] & 0xFF);
            break;
        }

         case 0b10111: { //movi
            reg[ro0] = imm;
            break;
         }

         case  0b11000: { // addi
            reg[ro0] = reg[ro0] + imm;
            break;
        }

        case  0b11001 : { //subi
            reg[ro0] = reg[ro0] - imm;
            break;
        }

        case 0b11010: {// muli
            reg[ro0] = reg[ro0] * imm;
            break;
            }

        case 0b11011: { //divi
            if (imm == 0) {
                erroCPU = 1;
                executando = 0;
                break;
            }
            reg[ro0] = reg[ro0] / imm;
            break;
        }

        case 0b11100  : { //lsh
            reg[ro0] = reg[ro0] << imm;
            break;
        }

        case 0b11101 : { //rsh
            reg[ro0] = reg[ro0] >> imm;
        break;
        }

        default: {
            erroCPU = 1;
            executando = 0;
            printf("\n erro de isntrucao, cpu = %d ", erroCPU);
            printf("\n executando = %d ", executando);
        }
    }
}


int main(int argc, char *argv[]) {
    inicializarCPU();

    if (argc < 2) {
        printf("uso: %s <arquivo.txt>\n", argv[0]);
        return 1;
    }

    carregarMemoriaArquivo(argv[1]);

    printf("--- TESTE DE BUSCA E DECODIFICACAO ---\n");

    while (executando == 1) {
        busca();

        if (erroCPU == 1) {
            break;
        }

        decodifica();

        if (erroCPU == 1) {
            break;
        }

        executa();

        // atualiza o pc so se nao foi um desvio (jumps zeram tamanhoInstrucao)
        pc = (pc + tamanhoInstrucao) & 0x00FF;

        exibirEstado();

        if (!executando) break;

        printf("\nPressione Enter para continuar...\n");
        getchar();
    }
    printf("\n--- FIM DO TESTE ---\n");
    return 0;
}