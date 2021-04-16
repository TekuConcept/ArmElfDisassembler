/**
 * Created by TekuConcept on March 10, 2020
 */

#ifndef ARM_DISASSEMBLER_H
#define ARM_DISASSEMBLER_H

#include <string>
#include <vector>

std::string disassemble(std::string binary);
std::vector<std::string> disassemble2array(std::string binary);

#endif
