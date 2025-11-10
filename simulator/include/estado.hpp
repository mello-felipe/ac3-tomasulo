#ifndef ESTADO_HPP
#define ESTADO_HPP

#include "types.hpp"
#include <vector>
#include <map>
#include <string>
#include <optional>

class Estado {
public:
    ConfigSimulador config;
    std::vector<EstadoInstrucao> estadoInstrucoes;
    std::map<std::string, UnidadeFuncional> unidadesFuncionais;
    std::map<std::string, UnidadeFuncionalMemoria> unidadesFuncionaisMemoria;
    int clock_cycle;
    std::map<std::string, std::optional<std::string>> estacaoRegistradores;

    Estado(const ConfigSimulador& cfg, const std::vector<InstrucaoInput>& instrucoes_input); // inicialização das instruções, registradores e unidades funcionais

    EstadoInstrucao* getNovaInstrucao(); // retorna a próxima instrução que ainda foi emitida 

    std::string verificaUFInstrucao(const InstrucaoDetalhes& instr_details); //mapeia a operação para a unidade funcional capaz de executá-la

    UnidadeFuncional* getFUVaziaArithInt(const std::string& tipoFU); // retorna ponteiro para a unidade funcional livre

    UnidadeFuncionalMemoria* getFUVaziaMem(const std::string& tipoFU);

    int getCiclos(const InstrucaoDetalhes& instr_details); //retorna o número de ciclos que a instrução gasta

    void alocaFU(UnidadeFuncional& uf, const InstrucaoDetalhes& instr_details, EstadoInstrucao& estado_instr_orig); // aloca uma unidade funcional para a instrução

    void alocaFuMem(UnidadeFuncionalMemoria& uf_mem, const InstrucaoDetalhes& instr_details, EstadoInstrucao& estado_instr_orig); // aloca uma unidade funcional para a instrução

    void escreveEstacaoRegistrador(const InstrucaoDetalhes& instr_details, const std::string& ufNome); // informa ao registrador final qual unidade funcional irá lhe entregar o resultado da operação

    void liberaUFEsperandoResultado(const std::string& nomeUFQueTerminou); // libera dependeências que estavam esperando a liberação da unidade funcional

    void desalocaUFMem(UnidadeFuncionalMemoria& uf_mem); // limpeza das instruções e mudança dos status da unidades funcionais

    void desalocaUF(UnidadeFuncional& uf);

    bool verificaSeJaTerminou(); // retorna true se todas as instruções do arquivo de entrada tiverem escrito seus resultados

    void issueNovaInstrucao(); // busca a nova instrução, procura uma unidade funcional para alocá-la e marca o ciclo de emissão da instrução

    void executaInstrucao(); // verifica se a unidade funcional não tem dependências, decrementa o tempo restante de execução e marca o ciclo de término da instrução

    void escreveInstrucao(); // registra o resultado da instrução em seu registrador de destino

    bool executa_ciclo(); // exxecuta um ciclo completo

    void printEstadoDebug() const; // imprime o estado das instruções, unidades funcionais, memória e registradores
};

#endif