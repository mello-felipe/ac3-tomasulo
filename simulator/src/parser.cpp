#include "parser.hpp"
#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

bool parseInputFile(const std::string& filename, ConfigSimulador& out_config, std::vector<InstrucaoInput>& out_instructions) { // leitura do arquivo de entrada
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    std::string line;
    enum class ParseState { NONE, CONFIG, INSTRUCTIONS };
    ParseState currentState = ParseState::NONE;

    while (std::getline(infile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line == "CONFIG_BEGIN") {
            currentState = ParseState::CONFIG;
            continue;
        } else if (line == "CONFIG_END") {
            currentState = ParseState::NONE;
            continue;
        } else if (line == "INSTRUCTIONS_BEGIN") {
            currentState = ParseState::INSTRUCTIONS;
            continue;
        } else if (line == "INSTRUCTIONS_END") {
            currentState = ParseState::NONE;
            break;
        }

        std::stringstream ss(line);
        std::string keyword, param1, param2_str;
        int param2_val;

        if (currentState == ParseState::CONFIG) {
            ss >> keyword;
            if (keyword == "CYCLES") {
                ss >> param1 >> param2_val;
                out_config.ciclos[param1] = param2_val;
            } else if (keyword == "UNITS") {
                ss >> param1 >> param2_val;
                out_config.unidades[param1] = param2_val;
            } else if (keyword == "MEM_UNITS") {
                ss >> param1 >> param2_val;
                out_config.unidadesMem[param1] = param2_val;
            } else {
                std::cerr << "Warning: Unknown config keyword '" << keyword << "' in line: " << line << std::endl;
            }
        } else if (currentState == ParseState::INSTRUCTIONS) {
            InstrucaoInput instr;
            ss >> instr.d_operacao >> instr.r_reg >> instr.s_reg_or_imm >> instr.t_reg_or_label;
            if (!instr.d_operacao.empty()) {
                out_instructions.push_back(instr);
            } else {
                std::cerr << "Warning: Could not parse instruction line: " << line << std::endl;
            }
        }
    }
    out_config.numInstrucoes = out_instructions.size();
    return true;
}