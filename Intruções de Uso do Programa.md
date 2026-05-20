
Dupla:
Filipe Divino Silva de Azevedo
Vanessa Gabrielle S. Santos

Definição do projeto:
  Simulador de uma máquina baseada na arquitetura de Von Neumann,
que efetua o ciclo de busca, decodificação e execução
  A CPU possui memória de 256 bytes, registradores de uso geral,
registradores auxiliares e executa instruções lidas de um arquivo
de texto.


Estrutura:
R0 a R7   registradores de uso geral de 16 bits
PC        aponta para a próxima instrução a ser executada
IR        guarda o opcode da instrução atual
MAR       guarda o endereço de memória acessado
MBR       guarda o dado lido ou escrito na memória
IMM       guarda o valor imediato ou endereço da instrução
RO0       índice do primeiro registrador da instrução
RO1       índice do segundo registrador da instrução
E         flag de igualdade
L         flag de menor
G         flag de maior


Memória:
A memória é representada por um vetor de 256 posições:

unsigned char memoria[256];

Dados de 16 bits são armazenados em big endian, o byte mais
significativo primeiro.

Funçoes:

inicializarCPU()
  Inicializa a CPU antes da execução. Coloca 0xFF em toda a memória,
  0xFFFF nos registradores gerais, PC em 0x0000 e liga a variável
  executando.

lerByteMemoria(endereco)
  Lê 1 byte da memória no endereço informado.
  O endereço é mascarado com 0x00FF antes do acesso.
  A função atualiza MAR e MBR.

escreverByteMemoria(endereco, dado)
  Grava 1 byte na memória no endereço informado.
  O endereço é mascarado com 0x00FF antes do acesso.
  A função atualiza MAR e MBR.

busca()
  Lê os bytes da instrução atual a partir do PC e coloca a instrução
  no MBR. Também identifica o tamanho da instrução, que pode ser de
  1, 2 ou 3 bytes.

decodifica()
  Decodifica o conteúdo do MBR, separando:
    IR  = opcode
    RO0 = primeiro registrador
    RO1 = segundo registrador
    IMM = imediato ou endereço de 16 bits

executa()
  Executa a instrução de acordo com o opcode armazenado em IR.

EstadoCPU()
  Imprime os registradores, flags, registradores auxiliares e toda
  a memória depois de cada ciclo de execução.

instrucaoParaOpcode(instrucao)
  Recebe o nome da instrução, como "add", "ld", "jmp", e retorna
  o opcode correspondente. Se a instrução não existir, retorna -1.

carregarArquivoMemoria(nomeArquivo)
  Lê o arquivo de entrada e carrega instruções e dados na memória.
  Cada linha do arquivo segue o formato:

    endereco;tipo;conteudo

  O tipo pode ser:
    i = instrução
    d = dado

Instruções:

As instruções podem ter 1, 2 ou 3 bytes.

Instruções de 1 byte:
  Primeiro byte:
    bits 7 a 3 = opcode
    bits 2 a 0 = registrador ou zero

Instruções de 2 bytes:
  Primeiro byte:
    bits 7 a 3 = opcode
    bits 2 a 0 = RO0

  Segundo byte:
    bits 7 a 5 = RO1
    bits 4 a 0 = zero

Instruções de 3 bytes:
  Primeiro byte:
    bits 7..3 = opcode
    bits 2..0 = RO0 ou zero

  Segundo e terceiro bytes:
    imm ou endereço de 16 bits


Conjunto de instrução suportado:
1 byte:
  hlt          para a execução
  nop          não faz nada
  not rX       inverte os bits de rX

2 bytes:
  ldr  rX, rY  rX = mem[rY]
  str  rX, rY  mem[rY] = rX
  add  rX, rY  rX = rX + rY
  sub  rX, rY  rX = rX - rY
  mul  rX, rY  rX = rX * rY
  div  rX, rY  rX = rX / rY
  cmp  rX, rY  compara e atualiza as flags E, L, G
  movr rX, rY  rX = rY
  and  rX, rY  rX = rX & rY
  or   rX, rY  rX = rX | rY
  xor  rX, rY  rX = rX ^ rY

3 bytes - jumps:
  je  Z        jump se E=1
  jne Z        jump se E=0
  jl  Z        jump se L=1
  jle Z        jump se L=1 ou E=1
  jg  Z        jump se G=1
  jge Z        jump se G=1 ou E=1
  jmp Z        jump sempre é realizado

3 bytes - memória e imm:
  ld   rX, Z    rX = mem[Z]
  st   rX, Z    mem[Z] = rX
  movi rX, imm  rX = IMM
  addi rX, imm  rX = rX + imm
  subi rX, imm  rX = rX - imm
  muli rX, imm  rX = rX * imm
  divi rX, imm  rX = rX / imm
  lsh  rX, imm  rX = rX << imm
  rsh  rX, imm  rX = rX >> imm
  
Erros durante a execuçao:

Arquivo não encontrado
  O programa imprime uma mensagem de erro e não inicia a execução.

Opcode errado
  O programa imprime o PC e o byte encontrado, seta erroCPU = 1
  e encerra a execução.

Instrução errada
  O carregador imprime uma mensagem de erro e cancela o carregamento
  do arquivo.


Instruçoes para compilação e uso do programa:

No terminal, dentro da pasta do projeto, execute o comando abaixo, ele ira compilar o programa:

  "gcc -Wall -Wextra -std=c11 main.c -o SimuladorCPU-MaquinaDeVonNeuman"

Deve ficar como abaixo no diretorio:
  "SimuladorCPU-MaquinaDeVonNeuman"


Para a execução deve se passar qual arquivo se deseja executar 
(O arquivo com o codigo a ser executado deve ser posto no diretorio "Programas")

./SimuladorCPU-MaquinaDeVonNeuman Programas/"arquivo.txt"

Exemplos:
./SimuladorCPU-MaquinaDeVonNeuman Programas/ProgramaExemplo.txt
./SimuladorCPU-MaquinaDeVonNeuman Programas/ProgramaExemplo2.tx
./SimuladorCPU-MaquinaDeVonNeuman Programas/Fibonacci.txt

Depois de cada ciclo, o programa imprime o estado da CPU e da memória.
Para continuar a execução, pressione Enter.

Também é possivel automatizar o processo de execução (para não precisar apertar enter a cada ciclo)
usando o parametro " yes""|" antes do comando de execução.

Exemplo:
"yes " " | ./SimuladorCPU-MaquinaDeVonNeuman Programas/ProgramaExemplo-2.txt"


Tipo de Programa suportado:

Cada linha do arquivo deve seguir o formato:
 "endereco;tipo;conteudo"
 
  endereco = endereço hexadecimal
  tipo = i para instrução, d para dados
  conteudo = instrução assembly ou dado hexadecimal

Exemplo dado no enunciado:
0;i;ld r0, 96
3;i;ld r1, 98
6;i;sub r0, r1
8;i;ld r1, 94
b;i;div r1, r0
d;i;ld r2, 92
10;i;mul r2, r1
12;i;ld r1, 90
15;i;add r1, r2
17;i;st r1, 8e
1a;i;hlt
90;d;20
92;d;3
94;d;4
96;d;5
98;d;3

Programa sorteado para a dupla - "1.Cálculo iterativo da sequência de Fibonacci com tamanho definido
em um valor de 16 bits armazenado em 0xFC e 0xFD. O resultado de 16 bits deve ser armazenado em 0xFE e 0xFF."    

Para modificar o valor calculado pelo programa Fibonacci, altere a linha
de dado responsável pela entrada "fc;d;a" (linha 23, a penultima linha)
Essa linha grava o valor 0x000A nos endereços 0xFC e 0xFD, como 0x000A equivale a 10 em decimal, o programa calcula Fibonacci(10).
Para calcular outro valor, basta trocar o dado hexadecimal.

Exemplos:
  "fc;d;0   calcula Fibonacci(0)
   fc;d;1   calcula Fibonacci(1)
   fc;d;2   calcula Fibonacci(2)
   fc;d;5   calcula Fibonacci(5)
   fc;d;a   calcula Fibonacci(10)"

O resultado será armazenado nos endereços 0xFE e 0xFF.

