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
                                                              // 00101 = sub                01010 = or
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

/*void exibirEstadoBuscaDecodifica(void) {
    printf("\nCPU apos busca e decodificacao:\n");
    printf("PC : 0x%04X\n", pc);
    printf("MAR: 0x%04X\n", mar);
    printf("MBR: 0x%08X\n", mbr);
    printf("IR : 0x%02X\n", ir);
    printf("RO0: 0x%X\n", ro0);
    printf("RO1: 0x%X\n", ro1);
    printf("IMM: 0x%04X\n", imm);
    printf("Tamanho da instrucao: %d byte\n", tamanhoInstrucao);
}
*/

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
            reg[ro0] = !reg[ro0];
            break;
        }

        case 0b01110: {
            // je z
            if (e == 1) {
                tamanhoInstrucao = 0;
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
            if (g == 1 || e == 1) {
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
            reg[ro0] = ((unsigned short int)byteAlto << 8) | byteBaixo;
            break;
        }

        case 0b10110: { //st
            escreverByteMemoria(imm,reg[ro0] >> 8);
            escreverByteMemoria(imm+1,reg[ro0] & 0xFF);
            break;
        }

        case 0b10111: { //movi
            reg[ro0] = imm;
            break;
        }

        case 0b11000: { // addi
            reg[ro0] = reg[ro0] + imm;
            break;
        }

        case 0b11001: { //subi
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

        case 0b11100: { //lsh

                reg[ro0] = reg[ro0] << imm;
          
            break;
        }

        case 0b11101: { //rsh

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

void EstadoCPU(void) {
    printf("\nCPU:\n");
    printf("R0: %04X R1: %04X R2: %04X R3: %04X\n",
           reg[0], reg[1], reg[2], reg[3]);
    printf("R4: %04X R5: %04X R6: %04X R7: %04X\n",
           reg[4], reg[5], reg[6], reg[7]);
    printf("MBR: %08X MAR: %04X IMM: %04X PC: %04X\n",
           mbr, mar, imm, pc);
    printf("IR: %02X RO0: %X RO1: %X\n", ir, ro0, ro1);
    printf("E: %X L: %X G: %X\n", e, l, g);

    printf("\nMemoria:\n");
    printf("   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");

    for (int linha = 0; linha < 16; linha++) {
        printf("%02X ", linha * 16);

        for (int coluna = 0; coluna < 16; coluna++) {
            printf("%02X ", memoria[linha * 16 + coluna]);
        }

        printf("\n");
    }
}

int instrucaoParaOpcode(char instrucao[]) {
    if (strcmp(instrucao, "hlt") == 0) return 0b00000;
    if (strcmp(instrucao, "nop") == 0) return 0b00001;
    if (strcmp(instrucao, "ldr") == 0) return 0b00010;
    if (strcmp(instrucao, "str") == 0) return 0b00011;
    if (strcmp(instrucao, "add") == 0) return 0b00100;
    if (strcmp(instrucao, "sub") == 0) return 0b00101;
    if (strcmp(instrucao, "mul") == 0) return 0b00110;
    if (strcmp(instrucao, "div") == 0) return 0b00111;
    if (strcmp(instrucao, "cmp") == 0) return 0b01000;
    if (strcmp(instrucao, "movr") == 0) return 0b01001;
    if (strcmp(instrucao, "and") == 0) return 0b01010;
    if (strcmp(instrucao, "or") == 0) return 0b01011;
    if (strcmp(instrucao, "xor") == 0) return 0b01100;
    if (strcmp(instrucao, "not") == 0) return 0b01101;
    if (strcmp(instrucao, "je") == 0) return 0b01110;
    if (strcmp(instrucao, "jne") == 0) return 0b01111;
    if (strcmp(instrucao, "jl") == 0) return 0b10000;
    if (strcmp(instrucao, "jle") == 0) return 0b10001;
    if (strcmp(instrucao, "jg") == 0) return 0b10010;
    if (strcmp(instrucao, "jge") == 0) return 0b10011;
    if (strcmp(instrucao, "jmp") == 0) return 0b10100;
    if (strcmp(instrucao, "ld") == 0) return 0b10101;
    if (strcmp(instrucao, "st") == 0) return 0b10110;
    if (strcmp(instrucao, "movi") == 0) return 0b10111;
    if (strcmp(instrucao, "addi") == 0) return 0b11000;
    if (strcmp(instrucao, "subi") == 0) return 0b11001;
    if (strcmp(instrucao, "muli") == 0) return 0b11010;
    if (strcmp(instrucao, "divi") == 0) return 0b11011;
    if (strcmp(instrucao, "lsh") == 0) return 0b11100;
    if (strcmp(instrucao, "rsh") == 0) return 0b11101;

    return -1;
}

int carregarArquivoMemoria(const char *arquivoDeInstrucao) {
    FILE *arquivo = fopen(arquivoDeInstrucao, "r");

    if (arquivo == NULL) {
        printf("\nERRO: nao foi possivel abrir o arquivo %s\n", arquivoDeInstrucao);
        return 0;
    }

    char linha[256];

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        unsigned int endereco = 0;
        unsigned int valor = 0;
        unsigned int r0 = 0;
        unsigned int r1 = 0;
        int opcode = -1;
        char tipo;
        char conteudo[256];
        char instrucao[20];
        char parametros[200];

        conteudo[0] = '\0';
        instrucao[0] = '\0';
        parametros[0] = '\0';
        linha[strcspn(linha, "\r\n")] = 0;

        if (linha[0] == '\0') {
            continue;
        }

        if (sscanf(linha, "%x;%c;%255[^\n]", &endereco, &tipo, conteudo) != 3) {
            printf("\nERRO: linha invalida: %s\n", linha);
            fclose(arquivo);
            return 0;
        }

        if (tipo == 'd') {
            sscanf(conteudo, "%x", &valor);
            escreverByteMemoria(endereco, (valor >> 8) & 0xFF);
            escreverByteMemoria(endereco + 1, valor & 0xFF);
        }
        else if (tipo == 'i') {
            sscanf(conteudo, "%19s %199[^\n]", instrucao, parametros);
            opcode = instrucaoParaOpcode(instrucao);

            if (opcode == -1) {
                printf("\nERRO: instrucao desconhecida: %s\n", instrucao);
                fclose(arquivo);
                return 0;
            }

            if (opcode == 0b00000 || opcode == 0b00001) {
                escreverByteMemoria(endereco, opcode << 3);
            }
            else if (opcode == 0b01101) {
                sscanf(parametros, "r%u", &r0);
                escreverByteMemoria(endereco, (opcode << 3) | (r0 & 0b00000111));
            }
            else if (opcode >= 0b00010 && opcode <= 0b01100) {
                sscanf(parametros, "r%u, r%u", &r0, &r1);
                escreverByteMemoria(endereco, (opcode << 3) | (r0 & 0b00000111));
                escreverByteMemoria(endereco + 1, (r1 & 0b00000111) << 5);
            }
            else if (opcode >= 0b01110 && opcode <= 0b10100) {
                sscanf(parametros, "%x", &valor);
                escreverByteMemoria(endereco, opcode << 3);
                escreverByteMemoria(endereco + 1, (valor >> 8) & 0xFF);
                escreverByteMemoria(endereco + 2, valor & 0xFF);
            }
            else if (opcode >= 0b10101 && opcode <= 0b11101) {
                sscanf(parametros, "r%u, %x", &r0, &valor);
                escreverByteMemoria(endereco, (opcode << 3) | (r0 & 0b00000111));
                escreverByteMemoria(endereco + 1, (valor >> 8) & 0xFF);
                escreverByteMemoria(endereco + 2, valor & 0xFF);
            }
        }
        else {
            printf("\nERRO: tipo invalido na linha: %s\n", linha);
            fclose(arquivo);
            return 0;
        }
    }

    fclose(arquivo);
    pc = 0x0000;

    return 1;
}

int main(void) {
    inicializarCPU();

    if (carregarArquivoMemoria("Programas/ProgramaExemplo.txt") == 0) {
        return 1;
    }

    printf("--- TESTE DA CPU ---\n");

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

        if (erroCPU == 1) {
            break;
        }

        if (executando == 1) {
            pc = (pc + tamanhoInstrucao) & 0x00FF;
        }

        //exibirEstadoBuscaDecodifica();
        EstadoCPU();

        if (executando == 1) {
            printf("\nPressione Enter para continuar...\n");
            getchar();
        }
    }

    printf("\n--- FIM DO PROGRAMA ---\n");
    return 0;
}