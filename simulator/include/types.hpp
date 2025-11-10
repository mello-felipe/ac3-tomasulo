#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <optional>
#include <map>

struct InstrucaoInput { // armazena a instrução lida no arquivo de entrada
    std::string d_operacao;
    std::string r_reg;
    std::string s_reg_or_imm;
    std::string t_reg_or_label;
};

struct ConfigSimulador {
    int numInstrucoes = 0;
    std::map<std::string, int> ciclos;
    std::map<std::string, int> unidades;
    std::map<std::string, int> unidadesMem;
};

struct InstrucaoDetalhes { // campos da instrução
    std::string operacao;
    std::string registradorR;
    std::string registradorS;
    std::string registradorT;
};

struct EstadoInstrucao { // guarda o estado de uma determinada instrução durante a execução do algoritmo
    InstrucaoDetalhes instrucao;
    int posicao;
    std::optional<int> issue;
    std::optional<int> exeCompleta;
    std::optional<int> write;
    bool busy = false;

    EstadoInstrucao() : posicao(0), busy(false) {}
    EstadoInstrucao(InstrucaoDetalhes details, int pos)
        : instrucao(std::move(details)), posicao(pos), busy(false) {}
};

struct UnidadeFuncional {
    std::optional<InstrucaoDetalhes> instrucao_details;
    EstadoInstrucao* estadoInstrucaoOriginal = nullptr;
    std::string tipoUnidade;
    std::optional<int> tempo;
    std::string nome;
    bool ocupado = false;
    std::optional<std::string> operacao;
    std::optional<std::string> vj;
    std::optional<std::string> vk;
    std::optional<std::string> qj;
    std::optional<std::string> qk;
};

struct UnidadeFuncionalMemoria {
    std::optional<InstrucaoDetalhes> instrucao_details;
    EstadoInstrucao* estadoInstrucaoOriginal = nullptr;
    std::string tipoUnidade;
    std::optional<int> tempo;
    std::string nome;
    bool ocupado = false;
    std::optional<std::string> qi;
    std::optional<std::string> qj;
    std::optional<std::string> operacao;
    std::optional<std::string> endereco;
    std::optional<std::string> destino;
};

#endif