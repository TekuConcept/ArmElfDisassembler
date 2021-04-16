#include "arm_decompiler.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <sstream>
#include <iterator>
#include "arm_disassembler.h"

struct function_symbol {
    unsigned int start;
    unsigned int size;
    std::string name;
    bool ispublic;
};

enum class condition_t {
    AL = 0, // always
    EQ,     // equal
    NE,     // not-equal
    CS,     // greater than or equal (unsigned)
    CC,     // less than (unsigned)
    MI,     // minus or negative result
    PL,     // positive or zero result
    VS,     // overflow
    VC,     // no overflow
    HI,     // high
    LS,     // low or same
    GE,     // greater than or equal
    LT,     // less than
    GT,     // greater than
    LE      // less than or equal
};

struct instruction_t {
    unsigned int byte_code;
    std::string name;
    condition_t condition;
    bool set;
    std::string comment;

    std::string store;
    std::vector<std::string> operands;
};

struct function_info {

};

struct section_info {
    std::string name;
};


static std::vector<function_symbol>
filter_function_symbols(
    const struct object& obj,
    size_t sidx)
{
    const auto& section = obj.sections[sidx];
    std::vector<function_symbol> functions;
    for (size_t j = 0; j < section.symbol_indices.size(); j++) {
        const auto& symbol = obj.symbols[section.symbol_indices[j]];
        const auto& name = obj.symbol_names[section.symbol_indices[j]];
        if (ELF32_ST_TYPE(symbol.st_info) != STT_FUNC) continue;
        
        auto f = std::find_if(functions.begin(), functions.end(),
        [&](const function_symbol& sym) { return sym.start == symbol.st_value; });
        if (f != functions.end()) {
            if (ELF32_ST_BIND(symbol.st_info) == STB_GLOBAL) {
                f->name = name;
                f->ispublic = true;
                assert(f->size == symbol.st_size);
            }
        }
        else {
            function_symbol func;
            func.name     = name;
            func.ispublic = ELF32_ST_BIND(symbol.st_info) == STB_GLOBAL;
            func.start    = symbol.st_value;
            func.size     = symbol.st_size;
            functions.push_back(func);
        }
    }
    std::sort(
        functions.begin(),
        functions.end(),
        [](const function_symbol& a, const function_symbol& b)
        { return a.start < b.start; }
    );
    return functions;
}


static bool
check_support(std::vector<std::string> string_instructions)
{
    std::set<std::string> ins;
    for (const auto& line : string_instructions) {
        if (line[0] == ' ' || line[0] == '\t') continue;
        auto start = line.find_first_of('\t');
        if (start == std::string::npos) {
            std::cout << "error: " << line << std::endl;
            return false;
        }
        ins.insert(line.substr(0, start));
    }
    for (const auto& i : ins)
        std::cout << i << "\n";
    return true;
}


void
parse_command(
    instruction_t& instruction,
    std::string str_command)
{
    std::cout << str_command << "\n";
}


static instruction_t
parse_instruction(const std::string& line)
{
    instruction_t instruction;

    std::string line2 = line;
    for (size_t i = 0; i < line2.size(); i++)
        if (line2[i] == '\t') line2[i] = ' ';
    std::istringstream iss(line2);
    std::vector<std::string> tokens(
        std::istream_iterator<std::string>{iss},
        std::istream_iterator<std::string>());

    // extract instruction, condition, and set values
    // adc, add, adr, and, asr, b, bfc, bfi, bic, bkpt,
    // bl, bx, cbz, cbnz, clz, cmp, cmn, eor, ldm, ldr,
    // lsl, lsr, mla, mls, mov, movt, mul, mvn, nop, orr,
    // pop, push, ror, rsb, rsc, sbc, sdiv, stm, str, sub,
    // tst, udiv, umlal, umull, uxtb, uxth

    if (tokens.size() == 0) {
        std::cout << "error: " << line << std::endl;
    }
    else {
        // parse_instruction(tokens[0]);
    }

    return instruction;
}


static section_info
decompile_section_at(
    const struct object& obj,
    size_t index)
{
    section_info section;
    section.name = obj.sections[index].name;

    std::vector<function_symbol> functions =
        filter_function_symbols(obj, index);
    if (functions.size() == 0) return section; // nothing to decompile

    std::vector<instruction_t> instructions;
    auto asm_strings = disassemble2array(obj.sections[index].raw_data);
    for (const auto& line : asm_strings)
        instructions.push_back(parse_instruction(line));

    // check_support(asm_strings);
    // std::cout << std::endl;

    return section;
}


void
decompile(struct object& obj)
{
    std::vector<section_info> decompiled_sections;

    // sort symbols
    for (size_t i = 0; i < obj.sections.size(); i++)
        ELF_sort_section_syms_by_value(obj, i);

    // find executable sections and decompile them
    for (size_t i = 0; i < obj.sections.size(); i++) {
        const auto& section = obj.sections[i];
        if (section.header.sh_type != SHT_PROGBITS) continue;
        if (!(section.header.sh_flags & SHF_EXECINSTR)) continue;
        decompiled_sections.push_back(decompile_section_at(obj, i));
    }
}
