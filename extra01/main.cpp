#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <set>

#include "arm_disassembler.h"


void print_usage(const char* program)
{ std::cout << "Usage: " << program << " <bin-file> [out-file]\n"; }



int translate(std::istream& in) {
    std::string raw = std::string(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());

    auto asm_strings = disassemble2array(raw);

    std::set<std::string> funcs;

    for (const auto& line : asm_strings) {
        if (line.size() <= 3) continue;
        if (line[0] != 'b' || line[1] != 'l' || line[2] != '\t')
            continue;
        funcs.insert(line);
    }

    unsigned int count;
    std::cout << std::hex;
    for (const auto& f : funcs) {
        std::cout << "func_" << count << ": ";
        std::cout << f << std::endl;
        count++;
    }
    std::cout << std::dec;

    return 0;
}



int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "No file!\n";
        print_usage(argv[0]);
        return 1;
    }

    std::ifstream in;

    in.open(argv[1], std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Failed to open file " << argv[1] << std::endl;
        return 1;
    }

    if (translate(in)) {
        in.close();
        std::cerr << "Could not translate hex to binary\n";
        return 1;
    }

    in.close();
    return 0;
}
