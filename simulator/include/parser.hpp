#ifndef PARSER_HPP
#define PARSER_HPP

#include "types.hpp"
#include <string>
#include <vector>

bool parseInputFile(const std::string& filename, ConfigSimulador& out_config, std::vector<InstrucaoInput>& out_instructions);

#endif