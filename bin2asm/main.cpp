/**
 * Created by TekuConcept on March 10, 2020
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

#include "arm_disassembler.h"


void print_usage(const char* program)
{
    std::cout << "Usage: " << program << " <bin-file> [out-file]\n\n";
    std::cout << program <<
        " takes a raw binary file such as a binary dump of a bootrom"
        " and disassembles it into pseudo-assembly";
    std::cout << std::endl;
}



int translate(std::istream& in, std::ostream& out) {
    std::string raw = std::string(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());

    auto asm_strings = disassemble2array(raw);

    unsigned int pc = 0;
    for (const auto& line : asm_strings) {
        out << std::setw(4) << std::hex << pc << ": ";
        out << line << "\n";
        pc += 4;
    }
    out << std::dec;

    return 0;
}



int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "No file!\n";
        print_usage(argv[0]);
        return 1;
    }
    else if (argc > 3) {
        std::cerr << "Too many arguments\n";
        print_usage(argv[0]);
        return 1;
    }

    std::string outfilename;
    std::ifstream in;
    std::ofstream out;

    if (argc == 3)
        outfilename = argv[2];
    else outfilename = "out.asm";

    in.open(argv[1], std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Failed to open file " << argv[1] << std::endl;
        return 1;
    }
    out.open(outfilename, std::ios::trunc);
    if (!out.is_open()) {
        in.close();
        std::cerr << "Failed to open file " << outfilename << std::endl;
        return 1;
    }

    if (translate(in, out)) {
        out.close();
        in.close();
        std::cerr << "Could not translate hex to binary\n";
        return 1;
    }

    out.close();
    in.close();
    return 0;
}
