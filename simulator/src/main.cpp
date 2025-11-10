#include "parser.hpp"
#include "estado.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) { //leitura do arquivo principal, criação do simulador, decisão de execução do algoritmo e executa todas as instruções
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.txt>" << std::endl;
        return 1;
    }
    std::string filename = argv[1];

    ConfigSimulador config;
    std::vector<InstrucaoInput> instructions;

    if (!parseInputFile(filename, config, instructions)) {
        return 1;
    }

    if (instructions.empty()) {
        std::cout << "No instructions found in the input file." << std::endl;
        return 0;
    }

    Estado simulador(config, instructions);
    bool terminou = false;
    int cycle_limit = 200;
    int current_cycle = 0;

    std::cout << "Simulacao Iniciada. Pressione Enter para avancar ciclo a ciclo, ou 'r' para rodar ate o fim." << std::endl;
    simulador.printEstadoDebug();

    char step_mode = 's';
    if (argc > 2 && std::string(argv[2]) == "run") {
        step_mode = 'r';
    }

    while (!terminou && current_cycle < cycle_limit) {
        if (step_mode == 's') {
            std::cout << "Pressione Enter para o proximo ciclo (Clock: " << simulador.clock_cycle + 1 << ") ou 'r' para rodar ate o fim: ";
            char c = std::cin.get();
            if (c == 'r' || c == 'R') {
                step_mode = 'r';
            }
            if (c != '\n' && c != EOF) {
                std::string dummy;
                std::getline(std::cin, dummy);
            }
        }
        terminou = simulador.executa_ciclo();
        current_cycle = simulador.clock_cycle;
        simulador.printEstadoDebug();

        if (terminou) {
            std::cout << "\n== Simulacao Concluida em " << simulador.clock_cycle << " ciclos. ==" << std::endl;
        }
    }

    if (!terminou && current_cycle >= cycle_limit) {
        std::cout << "\n== Simulacao Parada: Limite de ciclos (" << cycle_limit << ") atingido. ==" << std::endl;
    }

    std::cout << "\n== Estado Final dos Registradores Usados/Definidos ==" << std::endl;
    bool first_reg = true;
    for (const auto& pair : simulador.estacaoRegistradores) {
        if (!first_reg) std::cout << "; ";
        std::cout << pair.first << ": " << (pair.second.has_value() ? pair.second.value() : "initial/unused");
        first_reg = false;
    }
    std::cout << std::endl;

    return 0;
}