#include "estado.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

Estado::Estado(const ConfigSimulador& cfg, const std::vector<InstrucaoInput>& instrucoes_input) // inicialização das instruções, registradores e unidades funcionais
    : config(cfg), clock_cycle(0) {
    for (int i = 0; i < instrucoes_input.size(); ++i) {
        InstrucaoDetalhes details;
        details.operacao = instrucoes_input[i].d_operacao;
        details.registradorR = instrucoes_input[i].r_reg;
        details.registradorS = instrucoes_input[i].s_reg_or_imm;
        details.registradorT = instrucoes_input[i].t_reg_or_label;
        estadoInstrucoes.emplace_back(details, i);
    }
    this->config.numInstrucoes = instrucoes_input.size();

    for (const auto& pair : config.unidades) {
        const std::string& tipo = pair.first;
        int count = pair.second;
        for (int i = 0; i < count; ++i) {
            UnidadeFuncional uf;
            uf.nome = tipo + std::to_string(i + 1);
            uf.tipoUnidade = tipo;
            unidadesFuncionais[uf.nome] = uf;
        }
    }

    for (const auto& pair : config.unidadesMem) {
        const std::string& tipo = pair.first;
        int count = pair.second;
        for (int i = 0; i < count; ++i) {
            UnidadeFuncionalMemoria uf_mem;
            uf_mem.nome = tipo + std::to_string(i + 1);
            uf_mem.tipoUnidade = tipo;
            unidadesFuncionaisMemoria[uf_mem.nome] = uf_mem;
        }
    }

    for (int i = 0; i < 32; i += 2) {
        estacaoRegistradores["F" + std::to_string(i)] = std::nullopt;
    }
    for (int i = 0; i < 32; ++i) {
        estacaoRegistradores["R" + std::to_string(i)] = std::nullopt;
    }
}

EstadoInstrucao* Estado::getNovaInstrucao() { // retorna a próxima instrução que ainda foi emitida 
    for (auto& instr_state : estadoInstrucoes) {
        if (!instr_state.issue.has_value()) {
            return &instr_state;
        }
    }
    return nullptr;
}

std::string Estado::verificaUFInstrucao(const InstrucaoDetalhes& instr_details) { //mapeia a operação para a unidade funcional capaz de executá-la
    const std::string& op = instr_details.operacao;
    if (op == "ADDD" || op == "SUBD") return "Add";
    if (op == "MULTD") return "Mult";
    if (op == "DIVD") return "Div";
    if (op == "LD") return "Load";
    if (op == "SD") return "Store";
    if (op == "ADD" || op == "DADDUI" || op == "BEQ" || op == "BNEZ") return "Integer";
    std::cerr << "WARN: Unknown operation for UF check: " << op << std::endl;
    return "";
}

//getUFVazia: retorna ponteiro para a unidade funcional livre

UnidadeFuncional* Estado::getFUVaziaArithInt(const std::string& tipoFU) { 
    for (auto& pair : unidadesFuncionais) {
        if (pair.second.tipoUnidade == tipoFU && !pair.second.ocupado) {
            return &pair.second;
        }
    }
    return nullptr;
}

UnidadeFuncionalMemoria* Estado::getFUVaziaMem(const std::string& tipoFU) {
    for (auto& pair : unidadesFuncionaisMemoria) {
        if (pair.second.tipoUnidade == tipoFU && !pair.second.ocupado) {
            return &pair.second;
        }
    }
    return nullptr;
}

int Estado::getCiclos(const InstrucaoDetalhes& instr_details) { //retorna o número de ciclos que a instrução gasta
    const std::string& op_type = verificaUFInstrucao(instr_details);
    if (config.ciclos.count(op_type)) {
        return config.ciclos.at(op_type);
    }
    std::cerr << "Error: Cycle count not found for FU type '" << op_type
              << "' derived from operation '" << instr_details.operacao << "'" << std::endl;
    return 1;
}


void Estado::alocaFU(UnidadeFuncional& uf, const InstrucaoDetalhes& instr_details, EstadoInstrucao& estado_instr_orig) { // aloca uma unidade funcional para a instrução
    uf.instrucao_details = instr_details;
    uf.estadoInstrucaoOriginal = &estado_instr_orig;
    uf.tempo = getCiclos(instr_details) + 1;
    uf.ocupado = true;
    uf.operacao = instr_details.operacao;
    uf.vj = std::nullopt;
    uf.vk = std::nullopt;
    uf.qj = std::nullopt;
    uf.qk = std::nullopt;

    std::string src_reg_j_name, src_reg_k_name;

    if (instr_details.operacao == "ADDD" || instr_details.operacao == "SUBD") {
        if (uf.tempo.has_value() && uf.tempo.value() > 0) {
            uf.tempo = uf.tempo.value() - 1;
        }
    }

    if (instr_details.operacao == "BNEZ" || instr_details.operacao == "BEQ") {
        src_reg_j_name = instr_details.registradorR;
        src_reg_k_name = instr_details.registradorS;
    } else {
        src_reg_j_name = instr_details.registradorS;
        src_reg_k_name = instr_details.registradorT;
    }

    auto setup_operand = [&](std::optional<std::string>& v_val, std::optional<std::string>& q_val, const std::string& reg_name) {
        if (reg_name.empty()) {
            v_val = "N/A";
            return;
        }
        bool is_immediate = true;
        if (reg_name.empty() || (!reg_name.empty() && (reg_name[0] == 'F' || reg_name[0] == 'R'))) {
            is_immediate = false;
            for(char c : reg_name)
                if (reg_name.length() > 1 && isalpha(c) && c != 'F' && c != 'R')
                    is_immediate = true;
        }

        if (is_immediate && !(reg_name[0] == 'F' || reg_name[0] == 'R')) {
            v_val = reg_name;
        } else if (estacaoRegistradores.count(reg_name)) {
            const auto& reg_status = estacaoRegistradores.at(reg_name);
            if (!reg_status.has_value() || reg_status.value().rfind("VAL(", 0) == 0) {
                v_val = reg_status.has_value() ? reg_status.value() : reg_name;
            } else {
                const std::string& producing_fu_name = reg_status.value();
                if (unidadesFuncionais.count(producing_fu_name) || unidadesFuncionaisMemoria.count(producing_fu_name)) {
                    q_val = producing_fu_name;
                } else {
                    v_val = producing_fu_name;
                }
            }
        } else {
            v_val = reg_name;
        }
    };

    setup_operand(uf.vj, uf.qj, src_reg_j_name);
    setup_operand(uf.vk, uf.qk, src_reg_k_name);
}

void Estado::alocaFuMem(UnidadeFuncionalMemoria& uf_mem, const InstrucaoDetalhes& instr_details, EstadoInstrucao& estado_instr_orig) { // aloca uma unidade funcional para a instrução
    uf_mem.instrucao_details = instr_details;
    uf_mem.estadoInstrucaoOriginal = &estado_instr_orig;
    uf_mem.tempo = getCiclos(instr_details) + 1;
    uf_mem.ocupado = true;
    uf_mem.operacao = instr_details.operacao;
    uf_mem.endereco = instr_details.registradorS + "+" + instr_details.registradorT;
    uf_mem.destino = instr_details.registradorR;
    uf_mem.qi = std::nullopt;
    uf_mem.qj = std::nullopt;

    if (instr_details.operacao == "SD") {
        if (estacaoRegistradores.count(instr_details.registradorR)) {
            const auto& producing_fu_name_opt = estacaoRegistradores.at(instr_details.registradorR);
            if (producing_fu_name_opt.has_value()) {
                const std::string& producing_fu_name = producing_fu_name_opt.value();
                if (unidadesFuncionais.count(producing_fu_name) || unidadesFuncionaisMemoria.count(producing_fu_name)) {
                    uf_mem.qi = producing_fu_name;
                }
            }
        }
    }

    if (estacaoRegistradores.count(instr_details.registradorT)) {
        const auto& producing_fu_name_opt = estacaoRegistradores.at(instr_details.registradorT);
        if (producing_fu_name_opt.has_value()) {
            const std::string& producing_fu_name = producing_fu_name_opt.value();
            if (unidadesFuncionais.count(producing_fu_name) || unidadesFuncionaisMemoria.count(producing_fu_name)) {
                uf_mem.qj = producing_fu_name;
            }
        }
    }
}

void Estado::escreveEstacaoRegistrador(const InstrucaoDetalhes& instr_details, const std::string& ufNome) { //informa ao registrador final qual unidade funcional irá lhe entregar o resultado da operação
    if (instr_details.operacao != "SD" && instr_details.operacao != "BEQ" && instr_details.operacao != "BNEZ") {
        if (!instr_details.registradorR.empty()) {
            estacaoRegistradores[instr_details.registradorR] = ufNome;
        }
    }
}



void Estado::liberaUFEsperandoResultado(const std::string& nomeUFQueTerminou) { //libera dependeências que estavam esperando a liberação da unidade funcional
    std::string val_representation = "VAL(" + nomeUFQueTerminou + ")";

    for (auto& pair : unidadesFuncionais) {
        UnidadeFuncional& uf_esperando = pair.second;
        if (uf_esperando.ocupado) {
            bool qj_cleared_by_this_fu = false;
            bool qk_cleared_by_this_fu = false;

            if (uf_esperando.qj.has_value() && uf_esperando.qj.value() == nomeUFQueTerminou) {
                uf_esperando.vj = val_representation;
                uf_esperando.qj = std::nullopt;
                qj_cleared_by_this_fu = true;
            }
            if (uf_esperando.qk.has_value() && uf_esperando.qk.value() == nomeUFQueTerminou) {
                uf_esperando.vk = val_representation;
                uf_esperando.qk = std::nullopt;
                qk_cleared_by_this_fu = true;
            }

            if ((qj_cleared_by_this_fu || qk_cleared_by_this_fu) &&
                !uf_esperando.qj.has_value() && !uf_esperando.qk.has_value()) {
                if (uf_esperando.tempo.has_value() && uf_esperando.tempo.value() > 0) {
                    uf_esperando.tempo = uf_esperando.tempo.value() - 1;
                }
            }
        }
    }

    for (auto& pair : unidadesFuncionaisMemoria) {
        UnidadeFuncionalMemoria& uf_mem_esperando = pair.second;
        if (uf_mem_esperando.ocupado) {
            bool dependency_resolved_by_this_fu_for_mem = false;

            if (uf_mem_esperando.qi.has_value() && uf_mem_esperando.qi.value() == nomeUFQueTerminou) {
                uf_mem_esperando.qi = std::nullopt;
                dependency_resolved_by_this_fu_for_mem = true;
            }
            else if (uf_mem_esperando.qj.has_value() && uf_mem_esperando.qj.value() == nomeUFQueTerminou) {
                uf_mem_esperando.qj = std::nullopt;
                dependency_resolved_by_this_fu_for_mem = true;
            }

            if (dependency_resolved_by_this_fu_for_mem) {
                if (uf_mem_esperando.tempo.has_value() && uf_mem_esperando.tempo.value() > 0) {
                    uf_mem_esperando.tempo = uf_mem_esperando.tempo.value() - 1;
                }
            }
        }
    }
}

//limpeza das instruções e mudança dos status da unidades funcionais

void Estado::desalocaUFMem(UnidadeFuncionalMemoria& uf_mem) {
    uf_mem.instrucao_details = std::nullopt;
    uf_mem.estadoInstrucaoOriginal = nullptr;
    uf_mem.tempo = std::nullopt;
    uf_mem.ocupado = false;
    uf_mem.operacao = std::nullopt;
    uf_mem.endereco = std::nullopt;
    uf_mem.destino = std::nullopt;
    uf_mem.qi = std::nullopt;
    uf_mem.qj = std::nullopt;
}

void Estado::desalocaUF(UnidadeFuncional& uf) {
    uf.instrucao_details = std::nullopt;
    uf.estadoInstrucaoOriginal = nullptr;
    uf.tempo = std::nullopt;
    uf.ocupado = false;
    uf.operacao = std::nullopt;
    uf.vj = std::nullopt;
    uf.vk = std::nullopt;
    uf.qj = std::nullopt;
    uf.qk = std::nullopt;
}

bool Estado::verificaSeJaTerminou() { //retorna true se todas as instruções do arquivo de entrada tiverem escrito seus resultados
    if (estadoInstrucoes.empty()) return true;
    for (const auto& instr_state : estadoInstrucoes) {
        if (!instr_state.write.has_value()) {
            return false;
        }
    }
    return true;
}

void Estado::issueNovaInstrucao() { //busca a nova instrução, procura uma unidade funcional para alocá-la e marca o ciclo de emissão da instrução
    EstadoInstrucao* nova_instr_estado = getNovaInstrucao();
    if (nova_instr_estado) {
        std::string tipoFU_str = verificaUFInstrucao(nova_instr_estado->instrucao);
        if (tipoFU_str.empty()){
            std::cerr << "ERROR: Cannot determine FU type for " << nova_instr_estado->instrucao.operacao << std::endl;
            return;
        }

        if (tipoFU_str == "Load" || tipoFU_str == "Store") {
            UnidadeFuncionalMemoria* uf_para_usar = getFUVaziaMem(tipoFU_str);
            if (uf_para_usar) {
                alocaFuMem(*uf_para_usar, nova_instr_estado->instrucao, *nova_instr_estado);
                nova_instr_estado->issue = clock_cycle;
                if (uf_para_usar->instrucao_details.has_value() &&
                    uf_para_usar->instrucao_details.value().operacao != "SD") {
                    escreveEstacaoRegistrador(nova_instr_estado->instrucao, uf_para_usar->nome);
                }
            }
        } else {
            UnidadeFuncional* uf_para_usar = getFUVaziaArithInt(tipoFU_str);
            if (uf_para_usar) {
                alocaFU(*uf_para_usar, nova_instr_estado->instrucao, *nova_instr_estado);
                nova_instr_estado->issue = clock_cycle;
                const auto& op = nova_instr_estado->instrucao.operacao;
                if (op != "BEQ" && op != "BNEZ") {
                    escreveEstacaoRegistrador(nova_instr_estado->instrucao, uf_para_usar->nome);
                }
            }
        }
    }
}

void Estado::executaInstrucao() { //verifica se a unidade funcional não tem dependências, decrementa o tempo restante de execução e marca o ciclo de término da instrução
    for (auto& pair : unidadesFuncionaisMemoria) {
        UnidadeFuncionalMemoria& uf_mem = pair.second;
        if (uf_mem.ocupado && !uf_mem.qi.has_value() && !uf_mem.qj.has_value()) {
            if (uf_mem.tempo.has_value()) {
                if (uf_mem.tempo.value() > 0) {
                    uf_mem.tempo = uf_mem.tempo.value() - 1;
                    if (uf_mem.estadoInstrucaoOriginal)
                        uf_mem.estadoInstrucaoOriginal->busy = true;
                }
                if (uf_mem.tempo.value() == 0) {
                    if (uf_mem.estadoInstrucaoOriginal) {
                        uf_mem.estadoInstrucaoOriginal->exeCompleta = clock_cycle;
                        uf_mem.estadoInstrucaoOriginal->busy = false;
                        uf_mem.tempo = -1;
                    }
                }
            }
        }
    }

    for (auto& pair : unidadesFuncionais) {
        UnidadeFuncional& uf = pair.second;
        if (uf.ocupado && uf.vj.has_value() && uf.vk.has_value() &&
            !uf.qj.has_value() && !uf.qk.has_value()) {
            if (uf.tempo.has_value()) {
                if (uf.tempo.value() > 0) {
                    uf.tempo = uf.tempo.value() - 1;
                    if (uf.estadoInstrucaoOriginal)
                        uf.estadoInstrucaoOriginal->busy = true;
                }
                if (uf.tempo.value() == 0) {
                    if (uf.estadoInstrucaoOriginal) {
                        uf.estadoInstrucaoOriginal->exeCompleta = clock_cycle;
                        uf.estadoInstrucaoOriginal->busy = false;
                        uf.tempo = -1;
                    }
                }
            }
        }
    }
}

void Estado::escreveInstrucao() { //registra o resultado da instrução em seu registrador de destino
    for (auto& pair : unidadesFuncionaisMemoria) {
        UnidadeFuncionalMemoria& uf_mem = pair.second;
        if (uf_mem.ocupado && uf_mem.tempo.has_value() && uf_mem.tempo.value() == -1 &&
            uf_mem.estadoInstrucaoOriginal && !uf_mem.estadoInstrucaoOriginal->write.has_value() &&
            uf_mem.estadoInstrucaoOriginal->exeCompleta.has_value() &&
            uf_mem.estadoInstrucaoOriginal->exeCompleta.value() < clock_cycle) {
            uf_mem.estadoInstrucaoOriginal->write = clock_cycle;
            if (uf_mem.instrucao_details.has_value()) {
                const auto& instr_d = uf_mem.instrucao_details.value();
                if (instr_d.operacao != "SD") {
                    if (estacaoRegistradores.count(instr_d.registradorR)) {
                        auto& reg_status = estacaoRegistradores.at(instr_d.registradorR);
                        if (reg_status.has_value() && reg_status.value() == uf_mem.nome) {
                            reg_status = "VAL(" + uf_mem.nome + ")";
                        }
                    }
                }
            }
            liberaUFEsperandoResultado(uf_mem.nome);
            desalocaUFMem(uf_mem);
        }
    }

    for (auto& pair : unidadesFuncionais) {
        UnidadeFuncional& uf = pair.second;
        if (uf.ocupado && uf.tempo.has_value() && uf.tempo.value() == -1 &&
            uf.estadoInstrucaoOriginal && !uf.estadoInstrucaoOriginal->write.has_value() &&
            uf.estadoInstrucaoOriginal->exeCompleta.has_value() &&
            uf.estadoInstrucaoOriginal->exeCompleta.value() < clock_cycle) {
            uf.estadoInstrucaoOriginal->write = clock_cycle;
            if (uf.instrucao_details.has_value()) {
                const auto& instr_d = uf.instrucao_details.value();
                if (instr_d.operacao != "BEQ" && instr_d.operacao != "BNEZ") {
                    if (estacaoRegistradores.count(instr_d.registradorR)) {
                        auto& reg_status = estacaoRegistradores.at(instr_d.registradorR);
                        if (reg_status.has_value() && reg_status.value() == uf.nome) {
                            reg_status = "VAL(" + uf.nome + ")";
                        }
                    }
                }
            }
            liberaUFEsperandoResultado(uf.nome);
            desalocaUF(uf);
        }
    }
}

bool Estado::executa_ciclo() { //exxecuta um ciclo completo
    clock_cycle++;
    issueNovaInstrucao();
    executaInstrucao();
    escreveInstrucao();
    return verificaSeJaTerminou();
}

void Estado::printEstadoDebug() const { //imprime o estado das instruções, unidades funcionais, memória e registradores
    // Cabeçalho com estatísticas
    std::cout << "\n" << std::string(100, '=') << std::endl;
    std::cout << "  CLOCK CYCLE: " << clock_cycle;
    
    // Calcular estatísticas
    int emitidas = 0, executando = 0, completas = 0;
    for (const auto& s : estadoInstrucoes) {
        if (s.write.has_value()) {
            completas++;
        } else if (s.busy) {
            executando++;
        } else if (s.issue.has_value()) {
            emitidas++;
        }
    }
    
    std::cout << "  |  Emitidas: " << emitidas 
              << "  |  Executando: " << executando 
              << "  |  Completas: " << completas << " / " << estadoInstrucoes.size() << std::endl;
    std::cout << std::string(100, '=') << std::endl;
    
    // ========== TABELA 1: Status das Instruções ==========
    std::cout << "\n[ STATUS DAS INSTRUCOES ]" << std::endl;
    std::cout << std::string(100, '-') << std::endl;
    std::cout << std::left 
              << std::setw(6) << "ID"
              << std::setw(10) << "Operacao"
              << std::setw(8) << "Dest"
              << std::setw(10) << "Op1"
              << std::setw(10) << "Op2"
              << std::setw(8) << "Issue"
              << std::setw(8) << "Exec"
              << std::setw(8) << "Write"
              << std::setw(10) << "Busy" << std::endl;
    std::cout << std::string(100, '-') << std::endl;
    
    for (const auto& s : estadoInstrucoes) {
        std::cout << std::left 
                  << std::setw(6) << s.posicao
                  << std::setw(10) << s.instrucao.operacao
                  << std::setw(8) << (s.instrucao.registradorR.empty() ? "---" : s.instrucao.registradorR)
                  << std::setw(10) << (s.instrucao.registradorS.empty() ? "---" : s.instrucao.registradorS)
                  << std::setw(10) << (s.instrucao.registradorT.empty() ? "---" : s.instrucao.registradorT)
                  << std::setw(8) << (s.issue.has_value() ? std::to_string(s.issue.value()) : "---")
                  << std::setw(8) << (s.exeCompleta.has_value() ? std::to_string(s.exeCompleta.value()) : "---")
                  << std::setw(8) << (s.write.has_value() ? std::to_string(s.write.value()) : "---")
                  << std::setw(10) << (s.busy ? "[EXEC]" : "[ --- ]") << std::endl;
    }
    std::cout << std::string(100, '-') << std::endl;
    
    // ========== TABELA 2: Estações de Reserva (Aritmética/Inteiro) ==========
    std::cout << "\n[ ESTACOES DE RESERVA - Aritmetica/Inteiro ]" << std::endl;
    std::cout << std::string(110, '-') << std::endl;
    std::cout << std::left 
              << std::setw(12) << "Unidade"
              << std::setw(12) << "Status"
              << std::setw(8) << "Tempo"
              << std::setw(10) << "Operacao"
              << std::setw(14) << "Vj"
              << std::setw(14) << "Vk"
              << std::setw(12) << "Qj"
              << std::setw(12) << "Qk" << std::endl;
    std::cout << std::string(110, '-') << std::endl;
    
    for (const auto& pair : unidadesFuncionais) {
        const auto& uf = pair.second;
        std::string status = uf.ocupado ? "[OCUPADO]" : "[ LIVRE  ]";
        
        std::cout << std::left 
                  << std::setw(12) << uf.nome
                  << std::setw(12) << status
                  << std::setw(8) << (uf.tempo.has_value() ? std::to_string(uf.tempo.value()) : "---")
                  << std::setw(10) << (uf.operacao.has_value() ? uf.operacao.value() : "---")
                  << std::setw(14) << (uf.vj.has_value() ? uf.vj.value() : "---")
                  << std::setw(14) << (uf.vk.has_value() ? uf.vk.value() : "---")
                  << std::setw(12) << (uf.qj.has_value() ? uf.qj.value() : "---")
                  << std::setw(12) << (uf.qk.has_value() ? uf.qk.value() : "---") << std::endl;
    }
    std::cout << std::string(110, '-') << std::endl;
    
    // ========== TABELA 3: Buffers de Load/Store ==========
    std::cout << "\n[ BUFFERS DE LOAD/STORE ]" << std::endl;
    std::cout << std::string(110, '-') << std::endl;
    std::cout << std::left 
              << std::setw(12) << "Buffer"
              << std::setw(12) << "Status"
              << std::setw(8) << "Tempo"
              << std::setw(10) << "Op"
              << std::setw(18) << "Endereco"
              << std::setw(12) << "Reg"
              << std::setw(12) << "Qi"
              << std::setw(12) << "Qj (Base)" << std::endl;
    std::cout << std::string(110, '-') << std::endl;
    
    for (const auto& pair : unidadesFuncionaisMemoria) {
        const auto& uf = pair.second;
        std::string status = uf.ocupado ? "[OCUPADO]" : "[ LIVRE  ]";
        
        std::cout << std::left 
                  << std::setw(12) << uf.nome
                  << std::setw(12) << status
                  << std::setw(8) << (uf.tempo.has_value() ? std::to_string(uf.tempo.value()) : "---")
                  << std::setw(10) << (uf.operacao.has_value() ? uf.operacao.value() : "---")
                  << std::setw(18) << (uf.endereco.has_value() ? uf.endereco.value() : "---")
                  << std::setw(12) << (uf.destino.has_value() ? uf.destino.value() : "---")
                  << std::setw(12) << (uf.qi.has_value() ? uf.qi.value() : "---")
                  << std::setw(12) << (uf.qj.has_value() ? uf.qj.value() : "---") << std::endl;
    }
    std::cout << std::string(110, '-') << std::endl;
    
    // TABELA 4: Status dos Registradores (Grid Visual) 
    std::cout << "\n[ STATUS DOS REGISTRADORES ]" << std::endl;
    std::cout << std::string(100, '=') << std::endl;
    
    // Registradores Float (F0-F30, apenas pares)
    std::cout << "FLOAT (F0-F30):" << std::endl;
    std::cout << std::string(100, '-') << std::endl;
    
    int count = 0;
    for (int i = 0; i < 32; i += 2) {
        std::string reg_name = "F" + std::to_string(i);
        if (estacaoRegistradores.find(reg_name) != estacaoRegistradores.end()) {
            std::string valor = estacaoRegistradores.at(reg_name).has_value() 
                                ? estacaoRegistradores.at(reg_name).value() 
                                : "init";
            
            // Trunca valores muito longos para caber no grid
            if (valor.length() > 10) valor = valor.substr(0, 10);
            
            std::cout << std::setw(4) << std::right << reg_name << ": " 
                      << std::setw(12) << std::left << valor;
            
            count++;
            if (count % 5 == 0) {
                std::cout << std::endl;
            } else {
                std::cout << " | ";
            }
        }
    }
    if (count % 5 != 0) std::cout << std::endl;
    
    std::cout << std::string(100, '-') << std::endl;
    
    // Registradores Integer (R0-R31)
    std::cout << "INTEGER (R0-R31):" << std::endl;
    std::cout << std::string(100, '-') << std::endl;
    
    count = 0;
    for (int i = 0; i < 32; i++) {
        std::string reg_name = "R" + std::to_string(i);
        if (estacaoRegistradores.find(reg_name) != estacaoRegistradores.end()) {
            std::string valor = estacaoRegistradores.at(reg_name).has_value() 
                                ? estacaoRegistradores.at(reg_name).value() 
                                : "init";
            
            if (valor.length() > 10) valor = valor.substr(0, 10);
            
            std::cout << std::setw(4) << std::right << reg_name << ": " 
                      << std::setw(12) << std::left << valor;
            
            count++;
            if (count % 5 == 0) {
                std::cout << std::endl;
            } else {
                std::cout << " | ";
            }
        }
    }
    if (count % 5 != 0) std::cout << std::endl;
    
    std::cout << std::string(100, '=') << std::endl;
}