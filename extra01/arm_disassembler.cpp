/**
 * Created by TekuConcept on March 10, 2020
 */

#include "arm_disassembler.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include "libopcodes_config.h"
#include "dis-asm.h"


static int
disassemble_fprintf(
    void* stream,
    const char* fmt,
    ...)
{
    std::stringstream& os = *(std::stringstream*)stream;
    char* buffer;
    va_list arg;
    va_start(arg, fmt);
    vasprintf(&buffer, fmt, arg);
    std::string line(buffer);
    os << line;
    free(buffer);
    va_end(arg);
    return 0;
}


std::string
disassemble(std::string binary)
{
    std::stringstream ss;

    // STEP 1
    struct disassemble_info disasm_info;
    memset(&disasm_info, 0, sizeof(disassemble_info));
    init_disassemble_info(
        &disasm_info, &ss, (fprintf_ftype)disassemble_fprintf);
    disasm_info.arch = bfd_arch_arm;
    disasm_info.mach = bfd_mach_arm_unknown;
    disasm_info.read_memory_func = buffer_read_memory;
    disasm_info.buffer = (unsigned char*)&binary[0];
    disasm_info.buffer_vma = 0;
    disasm_info.buffer_length = binary.size();
    disassemble_init_for_target(&disasm_info);

    // STEP 2
    disassembler_ftype disasm;
    disasm = disassembler(
        disasm_info.arch,
        /*big-endian=*/false,
        disasm_info.mach,
        /*abfd=*/NULL
    );

    if (!disasm) {
        // std::cout << "No disassembler!" << std::endl;
        perror("No Disassembler\n");
        return "";
    }

    // STEP 3
    size_t pc = 0; // program counter
    while (pc < disasm_info.buffer_length) {
        size_t bytes_consumed = disasm(pc, &disasm_info);
        pc += bytes_consumed;
        ss << "\n";
    }

    return ss.str();
}


std::vector<std::string>
disassemble2array(std::string binary)
{
    std::vector<std::string> instructions;
    std::stringstream ss;

    // STEP 1
    struct disassemble_info disasm_info;
    memset(&disasm_info, 0, sizeof(disassemble_info));
    init_disassemble_info(
        &disasm_info, &ss, (fprintf_ftype)disassemble_fprintf);
    disasm_info.arch = bfd_arch_arm;
    disasm_info.mach = bfd_mach_arm_unknown;
    disasm_info.read_memory_func = buffer_read_memory;
    disasm_info.buffer = (unsigned char*)&binary[0];
    disasm_info.buffer_vma = 0;
    disasm_info.buffer_length = binary.size();
    disassemble_init_for_target(&disasm_info);

    // STEP 2
    disassembler_ftype disasm;
    disasm = disassembler(
        disasm_info.arch,
        /*big-endian=*/false,
        disasm_info.mach,
        /*abfd=*/NULL
    );

    if (!disasm) {
        // std::cout << "No disassembler!" << std::endl;
        perror("No Disassembler\n");
        return instructions;
    }

    // STEP 3
    size_t pc = 0; // program counter
    while (pc < disasm_info.buffer_length) {
        size_t bytes_consumed = disasm(pc, &disasm_info);
        pc += bytes_consumed;
        instructions.push_back(ss.str());
        ss.str("");
    }

    return instructions;
}
