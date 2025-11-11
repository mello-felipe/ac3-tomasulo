# Simulador do Algoritmo de Tomasulo

Este projeto implementa um simulador do algoritmo de Tomasulo para escalonamento dinâmico de instruções em processadores com execução fora de ordem.

---

## 🎯 Sobre o Projeto

O algoritmo de Tomasulo é uma técnica de escalonamento dinâmico que permite a execução fora de ordem de instruções, resolvendo dependências de dados através de estações de reserva e renomeação de registradores.

Este simulador demonstra:
- **Emissão (Issue)** de instruções para estações de reserva
- **Execução** quando as dependências são resolvidas
- **Escrita (Write)** dos resultados nos registradores de destino

---

## 🔨 Como Compilar

### Pré-requisitos
- Compilador C++ com suporte a C++17 (g++, clang++)
- Make (opcional, mas recomendado)

### Passos para compilação

```bash
# Clone ou navegue até o diretório do projeto
cd simulator/

# Compile usando Make
make

# Ou compile manualmente
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o simulator
```

Após a compilação, o executável `simulator` será criado no diretório raiz.

---

## 🚀 Como Executar

O simulador pode ser executado em dois modos:

### Modo Passo a Passo (Interativo)
Permite avançar ciclo por ciclo, visualizando o estado a cada etapa:

```bash
./simulator input.txt
```

Durante a execução:
- Pressione **Enter** para avançar para o próximo ciclo
- Digite **'r'** e pressione Enter para executar até o fim automaticamente

### Modo Execução Completa
Executa todos os ciclos automaticamente sem interrupção:

```bash
./simulator input.txt run
```

---

## 📄 Formato do Arquivo de Entrada

O arquivo de entrada (`input.txt`) define a configuração do simulador e as instruções a serem executadas.

### Estrutura do Arquivo

```
CONFIG_BEGIN
CYCLES <TipoUnidade> <NumCiclos>
UNITS <TipoUnidade> <Quantidade>
MEM_UNITS <TipoUnidade> <Quantidade>
CONFIG_END

INSTRUCTIONS_BEGIN
<OPERACAO> <RegDestino> <RegFonte1/Imm> <RegFonte2/Offset>
...
INSTRUCTIONS_END
```

### Seção CONFIG

Define as unidades funcionais e seus tempos de execução:

#### CYCLES - Tempo de Execução
Especifica quantos ciclos cada tipo de operação leva para executar:

```
CYCLES Add 2        # Adição/Subtração: 2 ciclos
CYCLES Mult 10      # Multiplicação: 10 ciclos
CYCLES Div 40       # Divisão: 40 ciclos
CYCLES Load 2       # Load da memória: 2 ciclos
CYCLES Store 2      # Store na memória: 2 ciclos
CYCLES Integer 1    # Operações inteiras: 1 ciclo
```

#### UNITS - Unidades Funcionais (Aritmética/Inteiro)
Define quantas unidades de cada tipo estão disponíveis:

```
UNITS Add 3         # 3 unidades de adição (Add1, Add2, Add3)
UNITS Mult 2        # 2 unidades de multiplicação (Mult1, Mult2)
UNITS Div 1         # 1 unidade de divisão (Div1)
UNITS Integer 2     # 2 unidades inteiras (Integer1, Integer2)
```

#### MEM_UNITS - Unidades de Memória
Define unidades para operações de carga e armazenamento:

```
MEM_UNITS Load 2    # 2 buffers de load (Load1, Load2)
MEM_UNITS Store 2   # 2 buffers de store (Store1, Store2)
```

### Seção INSTRUCTIONS

Lista as instruções a serem executadas em ordem:

#### Operações Suportadas

**Operações de Ponto Flutuante:**
```
ADDD  F0 F2 F4      # F0 = F2 + F4 (soma de doubles)
SUBD  F0 F2 F4      # F0 = F2 - F4 (subtração de doubles)
MULTD F0 F2 F4      # F0 = F2 * F4 (multiplicação de doubles)
DIVD  F0 F2 F4      # F0 = F2 / F4 (divisão de doubles)
```

**Operações de Memória:**
```
LD    F0 R1 100     # F0 = Mem[R1 + 100] (carrega da memória)
SD    F0 R1 100     # Mem[R1 + 100] = F0 (armazena na memória)
```

### Exemplo Completo de input.txt

```
CONFIG_BEGIN
CYCLES Add 2
CYCLES Mult 10
CYCLES Div 40
CYCLES Load 2
CYCLES Store 2
CYCLES Integer 1

UNITS Add 3
UNITS Mult 2
UNITS Div 1
UNITS Integer 2

MEM_UNITS Load 2
MEM_UNITS Store 2
CONFIG_END

INSTRUCTIONS_BEGIN
LD    F6  R2  0
LD    F2  R3  0
MULTD F0  F2  F4
SUBD  F8  F6  F2
DIVD  F10 F0  F6
ADDD  F6  F8  F2
INSTRUCTIONS_END
```

### Como Modificar o Arquivo de Entrada

1. **Alterar tempos de execução:** Modifique os valores em `CYCLES`
2. **Adicionar/remover unidades:** Ajuste os números em `UNITS` e `MEM_UNITS`
3. **Mudar instruções:** Edite a seção entre `INSTRUCTIONS_BEGIN` e `INSTRUCTIONS_END`
4. **Criar cenários de teste:** Experimente diferentes combinações de dependências

---

## ⚙️ Como Funciona

O simulador implementa o algoritmo de Tomasulo em três estágios principais por ciclo:

### 1. Issue (Emissão)
- Busca a próxima instrução ainda não emitida
- Verifica se há uma unidade funcional disponível do tipo necessário
- Aloca a unidade e registra as dependências (Qj, Qk para operações ou Qi, Qj para memória)
- Marca o ciclo de emissão da instrução
- Atualiza a estação de registradores para indicar qual UF produzirá o resultado

### 2. Execute (Execução)
- Para cada unidade funcional ocupada:
  - Verifica se todas as dependências foram resolvidas (Qj e Qk = null)
  - Se sim, decrementa o tempo restante de execução
  - Quando o tempo chega a 0, marca o ciclo de término da execução

### 3. Write (Escrita)
- Para instruções que terminaram a execução no ciclo anterior:
  - Escreve o resultado no registrador de destino
  - Libera dependências de outras instruções que esperavam este resultado
  - Converte Qj/Qk das instruções dependentes em valores disponíveis (Vj/Vk)
  - Desaloca a unidade funcional

### Resolução de Dependências

O simulador rastreia dependências através de:
- **Qj, Qk:** Nome da unidade funcional que produzirá o valor necessário
- **Vj, Vk:** Valor já disponível (registrador ou imediato)
- **Qi:** Para stores, indica dependência do dado a ser armazenado

Quando uma UF termina, o sistema:
1. Notifica todas as outras UFs que a esperavam
2. Converte suas dependências (Q) em valores disponíveis (V)
3. Permite que essas instruções comecem/continuem a executar

---

## 📊 Estrutura das Tabelas

A cada ciclo, o simulador imprime três tabelas principais:

### 1. Status das Instruções

Mostra o progresso de cada instrução através do pipeline:

```
#     Instr   R    S       T       Issue  Exec   Write  Busy
0     LD      F6   R2      0       1      3      4      Nao
1     LD      F2   R3      0       2      4      5      Nao
2     MULTD   F0   F2      F4      3      15     16     Nao
```

**Colunas:**
- **#:** Posição da instrução no programa
- **Instr:** Operação (LD, MULTD, ADDD, etc.)
- **R, S, T:** Operandos da instrução
- **Issue:** Ciclo em que foi emitida para uma unidade funcional
- **Exec:** Ciclo em que completou a execução
- **Write:** Ciclo em que escreveu o resultado
- **Busy:** Se está atualmente executando

### 2. Estações de Reserva (Unidades Aritméticas/Inteiro)

Mostra o estado das unidades funcionais não relacionadas à memória:

```
Nome      Ocupado Tempo  Op        Vj          Vk          Qj        Qk
Add1      Sim     1      ADDD      VAL(Add2)   F2          -         -
Mult1     Sim     8      MULTD     F2          F4          -         -
Div1      Nao     -      -         -           -           -         -
```

**Colunas:**
- **Nome:** Identificador da unidade (Add1, Mult1, etc.)
- **Ocupado:** Se está processando uma instrução
- **Tempo:** Ciclos restantes para completar (-1 = pronto para write)
- **Op:** Operação sendo executada
- **Vj, Vk:** Valores dos operandos (se disponíveis)
- **Qj, Qk:** Unidades funcionais que produzirão os operandos (dependências)

**Interpretação:**
- Se Qj ou Qk tem valor: instrução está esperando esse resultado
- Se Vj e Vk estão preenchidos e Qj/Qk vazios: instrução pode executar
- Tempo > 0: executando
- Tempo = -1: execução completa, aguardando write

### 3. Buffers de Load/Store (Unidades de Memória)

Mostra o estado das operações de memória:

```
Nome      Ocupado Tempo  Op      Endereco    Dest/Src  Qi        Qj (Base)
Load1     Sim     1      LD      R2+0        F6        -         -
Store1    Nao     -      -       -           -         -         -
```

**Colunas:**
- **Nome:** Identificador do buffer (Load1, Store1, etc.)
- **Ocupado:** Se está processando uma operação
- **Tempo:** Ciclos restantes para completar
- **Op:** LD (load) ou SD (store)
- **Endereco:** Cálculo do endereço (Base + Offset)
- **Dest/Src:** Registrador destino (LD) ou fonte (SD)
- **Qi:** Para SD, unidade que produzirá o dado a armazenar
- **Qj:** Dependência no registrador base do endereço

**Interpretação:**
- **Load (LD):** Qj indica se está esperando o endereço base
- **Store (SD):** Qi indica se está esperando o dado, Qj o endereço
- Ambos precisam ter dependências resolvidas antes de executar

---

## 📚 Exemplos

### Exemplo 1: Dependência RAW (Read After Write)

```
INSTRUCTIONS_BEGIN
ADDD  F0  F2  F4    # F0 = F2 + F4
MULTD F6  F0  F8    # F6 = F0 * F8 (depende de F0)
INSTRUCTIONS_END
```

A segunda instrução (MULTD) precisa esperar a primeira (ADDD) terminar:
- MULTD é emitida mas Qj aponta para Add1 (que está calculando F0)
- Só começa a executar quando Add1 completa e escreve F0
- Demonstra resolução automática de dependências

### Exemplo 2: Paralelismo

```
INSTRUCTIONS_BEGIN
ADDD  F0  F2  F4    # Independente
MULTD F6  F8  F10   # Independente
SUBD  F12 F14 F16   # Independente
INSTRUCTIONS_END
```

Como não há dependências, todas podem executar em paralelo:
- Se houver 3 unidades disponíveis, todas emitem no mesmo ciclo
- Executam simultaneamente
- Demonstra exploração de paralelismo em nível de instrução

### Exemplo 3: Operações de Memória

```
INSTRUCTIONS_BEGIN
LD    F2  R1  0     # Carrega F2 da memória
ADDD  F4  F2  F6    # Usa F2 (depende do LD)
SD    F4  R1  8     # Armazena F4 (depende do ADDD)
INSTRUCTIONS_END
```

Cadeia de dependências através da memória:
- ADDD espera LD completar (Qj = Load1)
- SD espera ADDD completar (Qi = Add1)
- Demonstra dependências em operações de memória

---

## 🛠️ Estrutura do Código

```
simulator/
├── include/
│   ├── types.hpp          # Definições de estruturas de dados
│   ├── estado.hpp         # Classe Estado (núcleo do simulador)
│   ├── parser.hpp         # Funções de parsing do arquivo
│   └── utils.hpp          # Funções utilitárias
├── src/
│   ├── estado.cpp         # Implementação do algoritmo
│   ├── parser.cpp         # Implementação do parser
│   ├── utils.cpp          # Implementação de utilitários
│   └── main.cpp           # Programa principal
└── Makefile               # Script de compilação
```

---

## 📖 Referências

- Tomasulo, R. M. (1967). "An Efficient Algorithm for Exploiting Multiple Arithmetic Units"
- Hennessy & Patterson - "Computer Architecture: A Quantitative Approach"

---