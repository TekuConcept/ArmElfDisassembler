/**
 * Created by TekuConcept on March 24, 2020
 */

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <set>

#include "elf_object.h"
#include "arm_disassembler.h"
#include "instruction.h"

using namespace arm;

elf_object obj;

int read_elf_file(std::string filename) {
    std::ifstream elf_file;
    elf_file.open(filename);

    if (!elf_file.is_open()) {
        std::cout << "Error opening file " << filename << "\n";
        return 1;
    }

    try { obj = elf_object::parse(elf_file); }
    catch (std::exception& e) {
        std::cout << "Error parsing elf file\n";
        std::cout << e.what() << std::endl;
        return 1;
    }

    elf_file.close();
    return 0;
}

void print_variable_data() {
    std::cout << "@\n@ -- FILES --\n@\n\n";
    for (const auto& symbol : obj.symbols())
        if (symbol.type == elf_object::sym_type_t::FILE)
            std::cout << "@ " << ((symbol.name.size() == 0) ? "[no name]" : symbol.name) << "\n";
    std::cout << "\n\n";

    for (const auto& section : obj.sections()) {
        if (section.name.find(".bss") != section.name.npos)
            std::cout << "unsigned char " << section.name.substr(1) << "[" << section.size << "];\n\n";
        else if (section.name.find(".data") != section.name.npos) {
            std::cout << "unsigned char " << section.name.substr(1);
            std::cout << "[" << section.size << "] = {";
            std::cout << std::hex << std::setfill('0');
            for (size_t i = 0; i < section.raw_data.size(); i++) {
                if (i % 16 == 0) std::cout << "\n    /*" << std::setw(4) << i << "*/ ";
                std::cout << "0x" << std::setw(2);
                std::cout << (int)(0xFF & section.raw_data[i]) << ", ";
            }
            std::cout << std::dec << std::setfill(' ');
            std::cout << "\n};\n\n";
        }
        else if (section.name.find(".rodata") != section.name.npos) {
            const auto& data = section.raw_data;
            auto caps_name = section.name;
            std::transform(caps_name.begin(), caps_name.end(), caps_name.begin(), ::toupper);
            std::replace(caps_name.begin(), caps_name.end(), '.', '_');

            std::ostringstream os;
            os << std::hex << std::setfill('0');
            os << caps_name << "_0000: .ascii \"";
            for (size_t i = 0; i < data.size(); i++) {
                if (data[i] >= ' ' && data[i] <= '~') {
                    if (data[i] == '\"') os << "\\\"";
                    else os << data[i];
                }
                else if (data[i] == '\n') os << "\\n";
                else if (data[i] == '\r') os << "\\r";
                else if (data[i] == '\t') os << "\\t";
                else if (data[i] == 0) {
                    os << "\\0";
                    if ((i % 4) == 3) {
                        os << "\",\n";
                        std::cout << os.str();
                        os.str("");
                        os << caps_name << "_" << std::setw(4) << (i + 1) << ": .ascii \"";
                    }
                }
                else os << "\\x" << std::setw(2) << (int)(0xFF & data[i]);
            }
            std::cout << os.str() << "\"\n\n\n";

            {
            // std::cout << "unsigned char " << section.name.substr(1);
            // std::cout << "[" << section.size << "] = {\n";
            // const auto& data = section.raw_data;
            // std::ostringstream os;
            // os << std::hex << std::setfill('0');
            // os << "    /*0000*/ \"";
            // for (size_t i = 0; i < data.size(); i++) {
            //     if (data[i] >= ' ' && data[i] <= '~') os << data[i];
            //     else if (data[i] == '\n') os << "\\n";
            //     else if (data[i] == '\r') os << "\\r";
            //     else if (data[i] == '\t') os << "\\t";
            //     else if (data[i] == 0) {
            //         os << "\\0";
            //         if ((i % 4) == 3) {
            //             os << "\",\n";
            //             std::cout << os.str();
            //             os.str("");
            //             os << "    /*" << std::setw(4) << (i + 1) << "*/ \"";
            //         }
            //     }
            //     else os << "\\x" << std::setw(2) << (int)(0xFF & data[i]);
            // }
            // std::cout << os.str() << "\"";
            // std::cout << "\n};\n\n";
            }
        }
    }
}

void print_formatted_assembly(unsigned int idx) {
    struct function_t {
        std::string name;
        unsigned int offset;
        std::vector<std::string> code;
    };

    const elf_object::section_t& section = obj.sections()[idx];
    if (section.raw_data.size() == 0) return;
    auto instructions = disassemble2array(section.raw_data);

    //
    // Comment relocation symbols
    //

    std::vector<std::string> leftovers;
    for (const auto& reloc : obj.relocations()) {
        if (reloc.sidx != idx) continue;
        if (reloc.symbol->name.size() == 0) continue;
        std::ostringstream os;
        auto k = reloc.offset >> 2;
        os << " @ " << reloc.symbol->name;
        os << " [" << elf_object::reloc_type_string(reloc.type) << ": ";
        os << (reloc.symbol->section ? reloc.symbol->section->name : "") << "+";
        os << "0x" << std::hex << reloc.symbol->value << "]";
        if (k >= instructions.size()) {
            os << " 0x" << reloc.offset;
            leftovers.push_back(os.str());
        }
        else instructions[k] += os.str();
    }

    //
    // Format Instructions
    //

    for (size_t i = 0; i < instructions.size(); i++) {
        std::replace(instructions[i].begin(), instructions[i].end(), ';', '@');
        std::replace(instructions[i].begin(), instructions[i].end(), '\t', ' ');

        std::istringstream iss(instructions[i]);
        std::vector<std::string> tokens(
            std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>());
        if (tokens.size() == 0) continue;

        std::ostringstream os;

        // instruction
        os << std::setw(12) << std::left << tokens[0] << std::right;

        for (size_t j = 1; j < tokens.size(); j++) {
            // comments
            if (tokens[j][0] == '@')
                os << std::setw(42 - os.str().size()) << "@ ";
            // arguments
            else os << tokens[j] << " ";
        }
        instructions[i] = os.str();

        // to uppercase
        auto k = instructions[i].find_first_of("@");
        std::string::iterator end = (k != std::string::npos) ?
            instructions[i].begin() + k : instructions[i].end();
        for (size_t k = 0; k < instructions[i].size(); k++) {
            if (instructions[i][k] == 'x') {
                if (k == 0 || instructions[i][k - 1] != '0')
                    instructions[i][k] = 'X';
            }
            else if (instructions[i][k] == '@') break;
            else instructions[i][k] = std::toupper(instructions[i][k]);
        }
    }

    //
    // apply select relocation symbols
    //

    for (const auto& reloc : obj.relocations()) {
        if (reloc.sidx != idx) continue;
        if (reloc.symbol->name.size() == 0) continue;
        auto k = reloc.offset >> 2;
        std::ostringstream os;
        if (k >= instructions.size()) continue;
        if (reloc.type == elf_object::reloc_type_t::R_ARM_CALL) {
            try {
                std::istringstream iss(instructions[k]);
                std::vector<std::string> tokens(
                    std::istream_iterator<std::string>{iss},
                    std::istream_iterator<std::string>());
                os << std::setw(12) << std::left << tokens[0] << std::right;
                os << reloc.symbol->name;
                instructions[k] = os.str();
            }
            catch (...) { }
        }
        else if (reloc.type == elf_object::reloc_type_t::R_ARM_MOVW_ABS_NC) {
            const auto& str = instructions[k];
            if (str[0] == 'M' && str[1] == 'O' && str[2] == 'V')
                instructions[k] = std::string("@ ") + instructions[k];
        }
        else if (reloc.type == elf_object::reloc_type_t::R_ARM_MOVT_ABS) {
            std::istringstream iss(instructions[k]);
            std::vector<std::string> tokens(
                std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>());
            const auto& str = tokens[0];
            if (str[0] == 'M' && str[1] == 'O' && str[2] == 'V') {
                auto caps_name = reloc.symbol->section->name;
                std::transform(caps_name.begin(), caps_name.end(), caps_name.begin(), ::toupper);
                std::replace(caps_name.begin(), caps_name.end(), '.', '_');
                os << std::setw(12) << std::left << "ADDRL" << std::right;
                os << tokens[1] << " =" << caps_name << "_";
                os << std::hex << std::setfill('0') << std::setw(4);
                os << reloc.symbol->value << " " << std::setfill(' ');
                os << std::setw(42 - os.str().size()) << "@ ";
                os << reloc.symbol->name;
                os << " [" << elf_object::reloc_type_string(reloc.type) << ": ";
                os << (reloc.symbol->section ? reloc.symbol->section->name : "") << "+";
                os << "0x" << std::hex << reloc.symbol->value << "]";
                instructions[k] = os.str();
            }
        }
    }

    //
    // special cases
    //

    for (size_t i = 0; i < instructions.size(); i++) {
        std::istringstream iss(instructions[i]);
        std::vector<std::string> tokens(
            std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>());
        const auto& ins = tokens[0];
        if (ins.size() >= 3 && ins[0] == 'L' && ins[1] == 'D' && ins[2] == 'R') {
            auto& t = tokens[2];
            if (t[0] == '[' && t[1] == 'P' && t[2] == 'C' && t[3] == ',') {
                std::ostringstream os;
                try {
                    const auto& addr = tokens[tokens.size() - 1];
                    size_t k = std::stoul(addr, nullptr, 16) >> 2;
                    if (k >= instructions.size()) continue;
                    os << "WORD_" << addr.substr(addr.size() - 4);
                    std::string label = os.str();
                    os << ": .word 0x" << std::hex << std::setfill('0') << std::setw(8);
                    os << *(uint32_t*)&section.raw_data[k << 2] << std::setfill(' ');
                    instructions[k] = os.str();
                    os.str("");
                    os << std::setw(12) << std::left << tokens[0] << std::right;
                    os << tokens[1] << " " << label << " ";
                    os << std::setw(42 - os.str().size()) << "@ " << instructions[i];
                    instructions[i] = os.str();
                } catch (...) { }
            }
        }
        else if (ins.size() >= 2 && ins[0] == 'B' && ins[1] == 'L') {
            std::ostringstream os;
            try {
                const auto& addr = tokens[tokens.size() - 1];
                size_t offset = std::stoul(addr, nullptr, 16);
                os << std::setw(12) << std::left << tokens[0] << std::right;
                for (const auto* symbol : section.symbols) {
                    if (symbol->type != elf_object::sym_type_t::FUNCTION) continue;
                    if (offset == symbol->value) {
                        os << symbol->name << " ";
                        os << std::setw(42 - os.str().size()) << "@ " << instructions[i];
                        instructions[i] = os.str();
                        break;
                    }
                }
            } catch (...) { }
        }
    }

    //
    // divide instructions into functions
    //

    std::vector<function_t> functions;
    for (const auto* symbol : section.symbols) {
        if (symbol->type != elf_object::sym_type_t::FUNCTION) continue;
        function_t function;
        function.name = symbol->name;
        function.offset = symbol->value;
        function.code.insert(
            function.code.end(),
            instructions.begin() + (symbol->value >> 2),
            instructions.begin() + ((symbol->value + symbol->size) >> 2)
        );
        functions.push_back(function);
    }

    //
    // sort functions
    //

    std::sort(functions.begin(), functions.end(),
    [](const function_t& a, const function_t& b) {
        return a.offset < b.offset;
    });

    //
    // insert jump labels
    //

    for (auto& function : functions) {
        std::set<std::pair<unsigned int,std::string>> labels;
        for (auto& instruction : function.code) {
            if (instruction[0] != 'B') continue;
            char c1 = instruction[1];
            char c2 = instruction[2];
            if ((c1 == ' ') ||
                (c1 == 'E' &&  c2 == 'Q') ||
                (c1 == 'N' &&  c2 == 'E') ||
                (c1 == 'C' && (c2 == 'S'  || c2 == 'C')) ||
                (c1 == 'H' && (c2 == 'S'  || c2 == 'I')) ||
                (c1 == 'L' && (c2 == 'O'  || c2 == 'S'   || c2 == 'T' || c2 == 'E')) ||
                (c1 == 'M' &&  c2 == 'I') ||
                (c1 == 'P' &&  c2 == 'L') ||
                (c1 == 'V' && (c2 == 'S'  || c2 == 'C')) ||
                (c1 == 'G' && (c2 == 'E'  || c2 == 'T')) ||
                (c1 == 'A' &&  c2 == 'L'))
            {
                std::istringstream iss(instruction);
                std::ostringstream label;
                std::ostringstream oss;
                std::vector<std::string> tokens(
                    std::istream_iterator<std::string>{iss},
                    std::istream_iterator<std::string>());
                if (tokens.size() < 2) continue;
                if (tokens[1].size() < 3 || (tokens[1][0] != '0' && tokens[1][1] != 'x')) continue;
                unsigned int offset = std::strtoul(tokens[1].data(), 0, 16);
                if (offset <   function.offset ||
                    offset >= (function.offset + (function.code.size() << 2)))
                    continue;
                label << function.name << "_x" << std::hex << offset;
                oss << std::setw(12) << std::left << tokens[0] << std::right << label.str();
                for (size_t j = 2; j < tokens.size(); j++) {
                    if (tokens[j][0] == '@')
                        oss << std::setw(42 - oss.str().size()) << "@ ";
                    else oss << tokens[j] << " ";
                }
                instruction = oss.str();
                labels.insert(std::pair<unsigned int, std::string>(
                    (offset - function.offset) >> 2, label.str()));
            }
        }
        std::for_each(labels.rbegin(), labels.rend(), [&](const std::pair<unsigned int,std::string>& label) {
            // assume label within function bounds
            function.code.insert(function.code.begin() + label.first, label.second + ":");
        });
    }

    //
    // print section info
    //

    std::cout << "@ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n";
    std::cout << "@ " << section.name << "\n";
    std::cout << "@ Size: 0x" << std::hex << section.raw_data.size() << std::dec << "\n";
    std::cout << "@ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n\n";

    //
    // print formatted functions
    //

    std::cout << std::hex << std::setfill('0');
    for (const auto& function : functions) {
        std::cout << "FUNC_BEGIN " << function.name << "\n";
        for (size_t i = 0, j = 0; i < function.code.size(); i++, j++) {
            const auto& code = function.code[i];
            if (code[code.size() - 1] != ':')
                std::cout << "    /*" << std::setw(8) << (function.offset + (j << 2)) << "*/ ";
            else {
                std::cout << "    ";
                j--;
            }
            std::cout << code << "\n";
        }
        std::cout << "FUNC_END " << function.name << "\n\n\n";
    }
    std::cout << std::dec << std::setfill(' ');

    // if (leftovers.size() > 0) {
    //     std::cout << "@ -- Leftover Relocations --\n\n";
    //     for (const auto& token : leftovers)
    //         std::cout << token << "\n";
    //     std::cout << "\n\n";
    // }
}

void print_formatted_c(unsigned int idx) {
    struct function_t {
        std::string name;
        unsigned int offset;
        std::vector<std::string> code;
    };

    const elf_object::section_t& section = obj.sections()[idx];
    if (section.raw_data.size() == 0) return;
    auto instructions = disassemble2array(section.raw_data);

    //
    // Apply relocation symbols
    //

    std::vector<std::string> leftovers;
    for (const auto& reloc : obj.relocations()) {
        if (reloc.sidx != idx) continue;
        if (reloc.symbol->name.size() == 0) continue;
        std::ostringstream os;
        auto idx = reloc.offset >> 2;
        os << " @ " << reloc.symbol->name;
        os << " [" << elf_object::reloc_type_string(reloc.type) << ": ";
        os << (reloc.symbol->section ? reloc.symbol->section->name : "") << "+";
        os << "0x" << std::hex << reloc.symbol->value << "]";
        if (idx >= instructions.size()) {
            os << " 0x" << reloc.offset;
            leftovers.push_back(os.str());
        }
        else instructions[idx] += os.str();
    }

    //
    // Format Instructions
    //

    for (size_t i = 0; i < instructions.size(); i++) {
        std::replace(instructions[i].begin(), instructions[i].end(), ';', '@');
        std::replace(instructions[i].begin(), instructions[i].end(), '\t', ' ');

        std::istringstream iss(instructions[i]);
        std::vector<std::string> tokens(
            std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>());
        if (tokens.size() == 0) continue;

        std::ostringstream os;

        // instruction
        os << std::setw(12) << std::left << tokens[0] << std::right;

        for (size_t j = 1; j < tokens.size(); j++) {
            // comments
            if (tokens[j][0] == '@')
                os << std::setw(42 - os.str().size()) << "@ ";
            // arguments
            else os << tokens[j] << " ";
        }
        instructions[i] = os.str();

        // to uppercase
        auto k = instructions[i].find_first_of("@");
        std::string::iterator end = (k != std::string::npos) ?
            instructions[i].begin() + k : instructions[i].end();
        for (size_t k = 0; k < instructions[i].size(); k++) {
            if (instructions[i][k] == 'x') {
                if (k == 0 || instructions[i][k - 1] != '0')
                    instructions[i][k] = 'X';
            }
            else if (instructions[i][k] == '@') break;
            else instructions[i][k] = std::toupper(instructions[i][k]);
        }
    }

    //
    // divide instructions into functions
    //

    std::vector<function_t> functions;
    for (const auto* symbol : section.symbols) {
        if (symbol->type != elf_object::sym_type_t::FUNCTION) continue;
        function_t function;
        function.name = symbol->name;
        function.offset = symbol->value;
        function.code.insert(
            function.code.end(),
            instructions.begin() + (symbol->value >> 2),
            instructions.begin() + ((symbol->value + symbol->size) >> 2)
        );
        functions.push_back(function);
    }

    //
    // sort functions
    //

    std::sort(functions.begin(), functions.end(),
    [](const function_t& a, const function_t& b) {
        return a.offset < b.offset;
    });

    //
    // insert jump labels
    //

    for (auto& function : functions) {
        std::set<std::pair<unsigned int,std::string>> labels;
        for (auto& instruction : function.code) {
            if (instruction[0] != 'B') continue;
            char c1 = instruction[1];
            char c2 = instruction[2];
            if ((c1 == ' ') ||
                (c1 == 'E' &&  c2 == 'Q') ||
                (c1 == 'N' &&  c2 == 'E') ||
                (c1 == 'C' && (c2 == 'S'  || c2 == 'C')) ||
                (c1 == 'H' && (c2 == 'S'  || c2 == 'I')) ||
                (c1 == 'L' && (c2 == 'O'  || c2 == 'S'   || c2 == 'T' || c2 == 'E')) ||
                (c1 == 'M' &&  c2 == 'I') ||
                (c1 == 'P' &&  c2 == 'L') ||
                (c1 == 'V' && (c2 == 'S'  || c2 == 'C')) ||
                (c1 == 'G' && (c2 == 'E'  || c2 == 'T')) ||
                (c1 == 'A' &&  c2 == 'L'))
            {
                std::istringstream iss(instruction);
                std::ostringstream label;
                std::ostringstream oss;
                std::vector<std::string> tokens(
                    std::istream_iterator<std::string>{iss},
                    std::istream_iterator<std::string>());
                if (tokens.size() < 2) continue;
                if (tokens[1].size() < 3 || (tokens[1][0] != '0' && tokens[1][1] != 'x')) continue;
                unsigned int offset = std::strtoul(tokens[1].data(), 0, 16);
                if (offset <   function.offset ||
                    offset >= (function.offset + (function.code.size() << 2)))
                    continue;
                label << function.name << "_x" << std::hex << offset;
                oss << std::setw(12) << std::left << tokens[0] << std::right << label.str();
                for (size_t j = 2; j < tokens.size(); j++) {
                    if (tokens[j][0] == '@')
                        oss << std::setw(42 - oss.str().size()) << "@ ";
                    else oss << tokens[j] << " ";
                }
                instruction = oss.str();
                labels.insert(std::pair<unsigned int, std::string>(
                    (offset - function.offset) >> 2, label.str()));
            }
        }
        std::for_each(labels.rbegin(), labels.rend(), [&](const std::pair<unsigned int,std::string>& label) {
            // assume label within function bounds
            function.code.insert(function.code.begin() + label.first, label.second + ":");
        });
    }

    //
    // print section info
    //

    std::cout << "// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n";
    std::cout << "// " << section.name << "\n";
    std::cout << "// Size: 0x" << std::hex << section.raw_data.size() << std::dec << "\n";
    std::cout << "// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n\n";

    //
    // print formatted functions
    //

    std::cout << std::hex << std::setfill('0');
    for (const auto& function : functions) {
        std::cout << "uint32_t " << function.name << "() {\n";
        for (size_t i = 0, j = 0; i < function.code.size(); i++, j++) {
            const auto& code = function.code[i];
            if (code[0] == '@') std::cout << "    // " << code << "\n";
            else if (code[code.size() - 1] != ':') {
                // std::cout << "    /*" << std::setw(8) << (function.offset + (j << 2)) << "*/ ";
                try {
                    auto ins = instruction_t::parse(code);
                    std::cout << "    " << ins.to_c() << "\n";
                } catch (std::runtime_error& e) {
                    std::cout << "Error converting instruction \"" << code << "\" to C\n";
                    std::cout << e.what() << std::endl;
                    exit(1);
                }
            }
            else {
                std::cout << "  " << code << "\n";
                j--;
            }
        }
        std::cout << "}\n\n\n";
    }
    std::cout << std::dec << std::setfill(' ');

    // if (leftovers.size() > 0) {
    //     std::cout << "@ -- Leftover Relocations --\n\n";
    //     for (const auto& token : leftovers)
    //         std::cout << token << "\n";
    //     std::cout << "\n\n";
    // }
}

void disassemble() {
    print_variable_data();

    const auto& sections = obj.sections();
    for (size_t i = 0; i < sections.size(); i++) {
        if (sections[i].type == elf_object::section_type_t::PROGBITS &&
            sections[i].name.rfind(".text") != sections[i].name.npos)
            print_formatted_assembly(i);
    }
}

void decompile() {
    print_variable_data();

    const auto& sections = obj.sections();
    for (size_t i = 0; i < sections.size(); i++) {
        if (sections[i].type == elf_object::section_type_t::PROGBITS &&
            sections[i].name.rfind(".text") != sections[i].name.npos)
            print_formatted_c(i);
    }
}

void print_header() {
    std::cout << std::hex;
    std::cout << "==========================\n";
    std::cout << "==      ELF HEADER      ==\n";
    std::cout << "==========================\n\n";
    std::cout << "Identity: "  << obj.id()      << "\n";
    std::cout << "Type: "      << obj.type()    << "\n";
    std::cout << "Machine: "   << obj.machine() << "\n";
    std::cout << "Version: 0x" << obj.version() << "\n";
    std::cout << "Entry: 0x"   << obj.entry()   << "\n";
    std::cout << "Flags: 0x"   << obj.flags()   << "\n";
    std::cout << "ELF Header Size: 0x"                   << obj.elf_header_size()                 << "\n";
    std::cout << "Section Header String Table Index: 0x" << obj.string_table_index()              << "\n";
    std::cout << "Section Header Table's Offset: 0x"     << obj.section_header_info().offset      << "\n";
    std::cout << "Section Header Table Entry Size: 0x"   << obj.section_header_info().entry_size  << "\n";
    std::cout << "Section Header Table Entry Count: 0x"  << obj.section_header_info().entry_count << "\n";
    std::cout << "Program Header Table's Offset: 0x"     << obj.program_header_info().offset      << "\n";
    std::cout << "Program Header Table Entry Size: 0x"   << obj.program_header_info().entry_size  << "\n";
    std::cout << "Program Header Table Entry Count: 0x"  << obj.program_header_info().entry_count << "\n";
    std::cout << std::dec << std::endl;
}

void print_sections() {
    const auto& sections = obj.sections();
    unsigned int index = 0;
    std::cout << std::hex;
    std::cout << "===============================\n";
    std::cout << "==      SECTION HEADERS      ==\n";
    std::cout << "===============================\n\n";
    std::cout << "Index | Name                           | ";
    std::cout << "Type                      | ";
    std::cout << "Flags    | Address  | Offset   | Size     | Link     | ";
    std::cout << "Info     | Align    | Entry Size \n";
    std::cout << "--------------------------------------------------";
    std::cout << "--------------------------------------------------";
    std::cout << "------------------------------------------------------------\n";
    for (const auto& section : sections) {
        std::cout << std::setw(5) << index++ << "   ";
        std::cout << std::setw(30) << section.name << "   ";
        std::cout << std::setw(25) << elf_object::section_type_string(section.type).substr(0, 25) << "   ";
        std::cout << std::setw( 8) << elf_object::section_flags_string(section.flags) << "   ";
        std::cout << std::setw( 8) << section.address    << "   ";
        std::cout << std::setw( 8) << section.offset     << "   ";
        std::cout << std::setw( 8) << section.size       << "   ";

        unsigned int link;
        for (size_t k = 0; k < sections.size(); k++) {
            if (section.link == &sections[k]) {
                link = k;
                break;
            }
        }

        std::cout << std::setw( 8) << link               << "   ";
        std::cout << std::setw( 8) << section.info       << "   ";
        std::cout << std::setw( 8) << section.align      << "   ";
        std::cout << std::setw( 8) << section.entry_size << "   ";
        std::cout << "\n";
    }
    std::cout << "--------------------------------------------------";
    std::cout << "--------------------------------------------------";
    std::cout << "------------------------------------------------------------\n";
    std::cout << std::dec << std::endl;
}

void print_symbols() {
    const auto& symbols = obj.symbols();
    const auto& sections = obj.sections();
    unsigned int count = 0;
    if (symbols.size() == 0) return;
    std::cout << std::hex;
    std::cout << "=======================\n";
    std::cout << "==      SYMBOLS      ==\n";
    std::cout << "=======================\n\n";
    std::cout << "Index | Value    | Size     | Bind    | Type    | Other | Section | ";
    std::cout << "Name                                \n";
    std::cout << "--------------------------------------------------";
    std::cout << "--------------------------------------------------\n";
    for (const auto& symbol : symbols) {
        std::cout << std::setw(5) << count         << "   ";
        std::cout << std::setw(8) << symbol.value  << "   ";
        std::cout << std::setw(8) << symbol.size   << "   ";
        std::cout << std::setw(7);
        switch (symbol.bind) {
        case elf_object::sym_bind_t::LOCAL:    std::cout << "LOCAL";   break;
        case elf_object::sym_bind_t::GLOBAL:   std::cout << "GLOBAL";  break;
        case elf_object::sym_bind_t::WEAK:     std::cout << "WEAK";    break;
        default:                               std::cout << "UNKNOWN"; break;
        }
        std::cout << "   " << std::setw(7);
        switch (symbol.type) {
        case elf_object::sym_type_t::NOTYPE:   std::cout << "NOTYPE";  break;
        case elf_object::sym_type_t::OBJECT:   std::cout << "OBJECT";  break;
        case elf_object::sym_type_t::FUNCTION: std::cout << "FUNC";    break;
        case elf_object::sym_type_t::SECTION:  std::cout << "SECTION"; break;
        case elf_object::sym_type_t::FILE:     std::cout << "FILE";    break;
        default:                               std::cout << "UNKNOWN"; break;
        }
        size_t sidx;
        for (size_t k = 0; k < sections.size(); k++) {
            if (&sections[k] == symbol.section) {
                sidx = k;
                break;
            }
        }
        std::cout << "   ";
        std::cout << std::setw(5) << (int)symbol.other << "   ";
        std::cout << std::setw(7) << sidx << "   ";
        std::cout << symbol.name << "\n";
    }
    std::cout << "--------------------------------------------------";
    std::cout << "--------------------------------------------------\n";
    std::cout << std::dec << std::endl;
}

void print_relocations() {
    const auto& relocations = obj.relocations();
    const auto& sections = obj.sections();
    unsigned int count = 0;
    if (relocations.size() == 0) return;
    std::cout << std::hex;
    std::cout << "===========================\n";
    std::cout << "==      RELOCATIONS      ==\n";
    std::cout << "===========================\n\n";
    std::cout << "Index | Offset   | Type                     | Symbol   \n";
    std::cout << "--------------------------------------------------";
    std::cout << "--------------------------------------------------\n";
    for (const auto& section : obj.sections()) {
        if (section.type != elf_object::section_type_t::REL) continue;
        std::cout << section.name << "\n__________\n";
        const auto& relocs = obj.relocations();
        for (const auto k : section.entry_indices) {
            std::cout << std::setw(5) << count++ << "   " << std::left;
            std::cout << std::setw(8) << relocs[k].offset << "   ";
            std::cout << std::setw(24) << elf_object::reloc_type_string(relocs[k].type) << "   ";

            std::string section_name;
            for (const auto& section : sections) {
                for (const auto& sym : section.symbols) {
                    if (relocs[k].symbol == sym) {
                        section_name = section.name;
                        break;
                    }
                }
                if (section_name.size() != 0) break;
            }

            std::cout << relocs[k].symbol->name << " ";
            std::cout << section_name << "@x" << relocs[k].symbol->value;

            std::cout << "\n" << std::right;
        }
        std::cout << "\n";
    }
    std::cout << "--------------------------------------------------";
    std::cout << "--------------------------------------------------\n";
    std::cout << std::dec << std::right << std::endl;
}

void print() {
    print_header();
    print_sections();
    print_symbols();
    print_relocations();
}

void print_usage(const char* program_name, bool error = true) {
    std::cout << "Usage: " << program_name << " [options] <elf-binary>\n";
    std::cout << "Options:\n";
    std::cout << "\t-d\tdisassembly binary into near-compilable assembly\n";
    std::cout << "\t-p\tprint all\n";
    std::cout << "\t-r\tprint relocations\n";
    std::cout << "\t-s\tprint sections\n";
    std::cout << "\t-t\tprint symbols\n";
    std::cout << "\t-h\tprint usage information\n";
    exit(error ? 1 : 0);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return -1;
    }

    if (read_elf_file(std::string(argv[argc - 1])) != 0)
        return 1;

    int opt;
    bool to_c = false;
    while((opt = getopt(argc, argv, "c:p:r:s:t:h")) != -1) {
        switch(opt) {
        case 'c': to_c = true; break;
        case 'p': print(); return 0;
        case 'r': print_relocations(); return 0;
        case 's': print_sections(); return 0;
        case 't': print_symbols(); return 0;
        case 'h': print_usage(argv[0], optarg != nullptr); break;
        default: print_usage(argv[0]); break;
        }
    }

    if (to_c) decompile();
    else disassemble();

    return 0;
}
