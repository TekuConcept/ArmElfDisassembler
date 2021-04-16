#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <string>
#include "elf.h"
#include "elf_printer.h"


static std::string
ELF_section_header_type2str(Elf32_Word type)
{
    switch (type) {
        case SHT_NULL:               return "Null";
        case SHT_PROGBITS:           return "Program Bits";
        case SHT_SYMTAB:             return "Symbol Table";
        case SHT_STRTAB:             return "String Table";
        case SHT_RELA:               return "Relocation Entries (w/addends)";
        case SHT_HASH:               return "Symbol Hash Table";
        case SHT_DYNAMIC:            return "Dynamic Linking";
        case SHT_NOTE:               return "Note";
        case SHT_NOBITS:             return "No-Bits";
        case SHT_REL:                return "Relocation Entries";
        case SHT_SHLIB:              return "<reserved>";
        case SHT_DYNSYM:             return "Dynamic Symbols";
        case SHT_ARM_EXIDX:          return "Arm Exception Index Table";
        case SHT_ARM_PREEMPTMAP:     return "BPABI DLL dynamic linking pre-emption map";
        case SHT_ARM_ATTRIBUTES:     return "Arm Compatibility attributes";
        case SHT_ARM_DEBUGOVERLAY:   return "Arm Debug Overlay";
        case SHT_ARM_OVERLAYSECTION: return "Arm Overlay Section";
        default: {
            std::ostringstream os;
            os << "Unknown (0x" << std::hex << type << ")";
            return os.str();
        } break;
    }
}


static std::string
ELF_section_flags2str(Elf32_Word flags)
{
    std::string str;
    if (flags & SHF_WRITE)            str.push_back('W');
    if (flags & SHF_ALLOC)            str.push_back('A');
    if (flags & SHF_EXECINSTR)        str.push_back('X');
    if (flags & 0x08)                 str.push_back('_');
    if (flags & SHF_MERGE)            str.push_back('M');
    if (flags & SHF_STRINGS)          str.push_back('S');
    if (flags & SHF_INFO_LINK)        str.push_back('I');
    if (flags & SHF_LINK_ORDER)       str.push_back('L');
    if (flags & SHF_OS_NONCONFORMING) str.push_back('O');
    if (flags & SHF_GROUP)            str.push_back('G');
    if (flags & SHF_TLS)              str.push_back('T');
    if (flags & (~0x7FF))             str.push_back('?');
    return str;
}


static void
ELF_print_header(const struct object& obj)
{
    auto& hdr = obj.header;
    std::cout << "==========================\n";
    std::cout << "==      ELF HEADER      ==\n";
    std::cout << "==========================\n\n";
    std::cout << "Identity: " << std::setfill('0');
    for (size_t i = 0; i < EI_NIDENT; i++)
        std::cout << std::hex << std::setw(2) << (int)hdr.e_ident[i];
    std::cout << std::endl;
    std::cout << "Type: ";
    switch (hdr.e_type) {
        case ET_NONE: std::cout << "None\n"; break;
        case ET_REL: std::cout << "Relocatable\n"; break;
        case ET_EXEC: std::cout << "Executable\n"; break;
        case ET_DYN: std::cout << "Shared Object\n"; break;
        case ET_CORE: std::cout << "Core\n"; break;
        default: std::cout << "Unknown (0x" << std::hex << hdr.e_type << ")\n"; break;
    }
    std::cout << "Machine: ";
    switch (hdr.e_machine) {
        case EM_NONE: std::cout << "None\n"; break;
        case EM_M32: std::cout << "M32\n"; break;
        case EM_SPARC: std::cout << "SPARC\n"; break;
        case EM_386: std::cout << "386\n"; break;
        case EM_68K: std::cout << "68K\n"; break;
        case EM_88K: std::cout << "88K\n"; break;
        case EM_860: std::cout << "860\n"; break;
        case EM_MIPS: std::cout << "MIPS\n"; break;
        case EM_ARM: std::cout << "ARM\n"; break;
        default: std::cout << "Unknown (0x" << std::hex << hdr.e_machine << ")\n"; break;
    }
    std::cout << "Version: 0x" << std::hex << hdr.e_version << std::endl;
    std::cout << "Entry: 0x" << std::hex << hdr.e_entry << std::endl;
    std::cout << "Program Header Table's Offset: 0x" << std::hex << hdr.e_phoff << std::endl;
    std::cout << "Section Header Table's Offset: 0x" << std::hex << hdr.e_shoff << std::endl;
    std::cout << "Flags: 0x" << std::hex << hdr.e_flags << std::endl;
    std::cout << "ELF Header Size: 0x" << std::hex << hdr.e_ehsize << std::endl;
    std::cout << "Program Header Table Entry Size: 0x" << std::hex << hdr.e_phentsize << std::endl;
    std::cout << "Program Header Table Entry Count: 0x" << std::hex << hdr.e_phnum << std::endl;
    std::cout << "Section Header Table Entry Size: 0x" << std::hex << hdr.e_shentsize << std::endl;
    std::cout << "Section Header Table Entry Count: 0x" << std::hex << hdr.e_shnum << std::endl;
    std::cout << "Section Header String Table Index: 0x" << std::hex << hdr.e_shstrndx << std::endl;
    std::cout << std::dec << std::setfill(' ') << std::endl;
}


static void
ELF_print_section_headers(const struct object& obj)
{
    // std::cout << std::setfill('0');
    std::cout << "===============================\n";
    std::cout << "==      SECTION HEADERS      ==\n";
    std::cout << "===============================\n\n";
    std::cout << "Index | Name                           | ";
    std::cout << "Type                      | ";
    std::cout << "Flags    | Address  | Offset   | Size     | Link     | ";
    std::cout << "Info     | Align    | Entry Size \n";
    std::cout << "----------------------------------------";
    std::cout << "----------------------------";
    std::cout << "-------------------------------------------------------";
    std::cout << "---------------------------------\n";
    for (size_t i = 0; i < obj.section_headers.size(); i++) {
        const auto& hdr = obj.section_headers[i];
        std::cout << std::hex << std::setw(5) << i << "   ";
        std::cout << std::setw(30) << obj.section_names[i]                     << "   ";
        std::cout << std::setw(25) << ELF_section_header_type2str(hdr.sh_type).substr(0, 25) << "   ";
        std::cout << std::setw( 8) << ELF_section_flags2str(hdr.sh_flags)      << "   ";
        std::cout << std::setw( 8) << std::hex << hdr.sh_addr                  << "   ";
        std::cout << std::setw( 8) << std::hex << hdr.sh_offset                << "   ";
        std::cout << std::setw( 8) << std::hex << hdr.sh_size                  << "   ";
        std::cout << std::setw( 8) << std::hex << hdr.sh_link                  << "   ";
        std::cout << std::setw( 8) << std::hex << hdr.sh_info                  << "   ";
        std::cout << std::setw( 8) << std::hex << hdr.sh_addralign             << "   ";
        std::cout << std::setw( 8) << std::hex << hdr.sh_entsize               << "   ";
        std::cout << "\n";
    }
    std::cout << "----------------------------------------";
    std::cout << "----------------------------";
    std::cout << "-------------------------------------------------------";
    std::cout << "---------------------------------\n";
    std::cout << std::dec << std::setfill(' ') << std::endl;
}


void
ELF_print_symbols(
    const struct object& obj,
    const std::vector<size_t>& indices)
{
    if (indices.size() == 0) return;
    std::cout << std::setfill(' ');
    std::cout << "=======================\n";
    std::cout << "==      SYMBOLS      ==\n";
    std::cout << "=======================\n\n";
    std::cout << "Index | Value    | Size     | Bind    | Type    | Other | Section | ";
    std::cout << "Name                                \n";
    std::cout << "--------------------------------------------------------------------";
    std::cout << "------------------------------------\n";
    for (size_t i = 0; i < indices.size(); i++) {
        const auto& sym = obj.symbols[indices[i]];
        std::cout << std::hex << std::setw(5) << i                 << "   ";
        std::cout << std::setw(8) << std::hex << sym.st_value      << "   ";
        std::cout << std::setw(8) << std::hex << sym.st_size       << "   ";
        std::cout << std::setw(7);
        switch (ELF32_ST_BIND(sym.st_info)) {
        case STB_LOCAL:  std::cout << "LOCAL";   break;
        case STB_GLOBAL: std::cout << "GLOBAL";  break;
        case STB_WEAK:   std::cout << "WEAK";    break;
        default:         std::cout << "UNKNOWN"; break;
        }
        std::cout << "   " << std::setw(7);
        switch (ELF32_ST_TYPE(sym.st_info)) {
        case STT_NOTYPE:  std::cout << "NOTYPE";  break;
        case STT_OBJECT:  std::cout << "OBJECT";  break;
        case STT_FUNC:    std::cout << "FUNC";    break;
        case STT_SECTION: std::cout << "SECTION"; break;
        case STT_FILE:    std::cout << "FILE";    break;
        default:          std::cout << "UNKNOWN"; break;
        }
        std::cout << "   ";
        std::cout << std::setw(5) << std::hex << (int)sym.st_other << "   ";
        std::cout << std::setw(7) << std::hex << sym.st_shndx      << "   ";
        std::cout << obj.symbol_names[indices[i]]                  << "\n";
    }
    std::cout << "--------------------------------------------------------------------";
    std::cout << "------------------------------------\n";
    std::cout << std::dec << std::setfill(' ') << std::endl;
}


static std::string
ELF_relocation_type2str(int type)
{
    #define CASE_STRING(label) \
        case label: return #label;
    switch (type) {
    CASE_STRING(R_ARM_NONE                  )
    CASE_STRING(R_ARM_PC24                  )
    CASE_STRING(R_ARM_ABS32                 )
    CASE_STRING(R_ARM_REL32                 )
    CASE_STRING(R_ARM_LDR_PC_G0             )
    CASE_STRING(R_ARM_ABS16                 )
    CASE_STRING(R_ARM_ABS12                 )
    CASE_STRING(R_ARM_THM_ABS5              )
    CASE_STRING(R_ARM_ABS8                  )
    CASE_STRING(R_ARM_SBREL32               )
    CASE_STRING(R_ARM_THM_CALL              )
    CASE_STRING(R_ARM_THM_PC8               )
    CASE_STRING(R_ARM_BREL_ADJ              )
    CASE_STRING(R_ARM_TLS_DESC              )
    CASE_STRING(R_ARM_THM_SWI8              )
    CASE_STRING(R_ARM_XPC25                 )
    CASE_STRING(R_ARM_THM_XPC22             )
    CASE_STRING(R_ARM_TLS_DTPMOD32          )
    CASE_STRING(R_ARM_TLS_DTPOFF32          )
    CASE_STRING(R_ARM_TLS_TPOFF32           )
    CASE_STRING(R_ARM_COPY                  )
    CASE_STRING(R_ARM_GLOB_DAT              )
    CASE_STRING(R_ARM_JUMP_SLOT             )
    CASE_STRING(R_ARM_RELATIVE              )
    CASE_STRING(R_ARM_GOTOFF32              )
    CASE_STRING(R_ARM_BASE_PREL             )
    CASE_STRING(R_ARM_GOT_BREL              )
    CASE_STRING(R_ARM_PLT32                 )
    CASE_STRING(R_ARM_CALL                  )
    CASE_STRING(R_ARM_JUMP24                )
    CASE_STRING(R_ARM_THM_JUMP24            )
    CASE_STRING(R_ARM_BASE_ABS              )
    CASE_STRING(R_ARM_ALU_PCREL_7_0         )
    CASE_STRING(R_ARM_ALU_PCREL_15_8        )
    CASE_STRING(R_ARM_ALU_PCREL_23_15       )
    CASE_STRING(R_ARM_LDR_SBREL_11_0_NC     )
    CASE_STRING(R_ARM_ALU_SBREL_19_12_NC    )
    CASE_STRING(R_ARM_ALU_SBREL_27_20_CK    )
    CASE_STRING(R_ARM_TARGET1               )
    CASE_STRING(R_ARM_SBREL31               )
    CASE_STRING(R_ARM_V4BX                  )
    CASE_STRING(R_ARM_TARGET2               )
    CASE_STRING(R_ARM_PREL31                )
    CASE_STRING(R_ARM_MOVW_ABS_NC           )
    CASE_STRING(R_ARM_MOVT_ABS              )
    CASE_STRING(R_ARM_MOVW_PREL_NC          )
    CASE_STRING(R_ARM_MOVT_PREL             )
    CASE_STRING(R_ARM_THM_MOVW_ABS_NC       )
    CASE_STRING(R_ARM_THM_MOVT_ABS          )
    CASE_STRING(R_ARM_THM_MOVW_PREL_NC      )
    CASE_STRING(R_ARM_THM_MOVT_PREL         )
    CASE_STRING(R_ARM_THM_JUMP19            )
    CASE_STRING(R_ARM_THM_JUMP6             )
    CASE_STRING(R_ARM_THM_ALU_PREL_11_0     )
    CASE_STRING(R_ARM_THM_PC12              )
    CASE_STRING(R_ARM_ABS32_NOI             )
    CASE_STRING(R_ARM_REL32_NOI             )
    CASE_STRING(R_ARM_ALU_PC_G0_NC          )
    CASE_STRING(R_ARM_ALU_PC_G0             )
    CASE_STRING(R_ARM_ALU_PC_G1_NC          )
    CASE_STRING(R_ARM_ALU_PC_G1             )
    CASE_STRING(R_ARM_ALU_PC_G2             )
    CASE_STRING(R_ARM_LDR_PC_G1             )
    CASE_STRING(R_ARM_LDR_PC_G2             )
    CASE_STRING(R_ARM_LDRS_PC_G0            )
    CASE_STRING(R_ARM_LDRS_PC_G1            )
    CASE_STRING(R_ARM_LDRS_PC_G2            )
    CASE_STRING(R_ARM_LDC_PC_G0             )
    CASE_STRING(R_ARM_LDC_PC_G1             )
    CASE_STRING(R_ARM_LDC_PC_G2             )
    CASE_STRING(R_ARM_ALU_SB_G0_NC          )
    CASE_STRING(R_ARM_ALU_SB_G0             )
    CASE_STRING(R_ARM_ALU_SB_G1_NC          )
    CASE_STRING(R_ARM_ALU_SB_G1             )
    CASE_STRING(R_ARM_ALU_SB_G2             )
    CASE_STRING(R_ARM_LDR_SB_G0             )
    CASE_STRING(R_ARM_LDR_SB_G1             )
    CASE_STRING(R_ARM_LDR_SB_G2             )
    CASE_STRING(R_ARM_LDRS_SB_G0            )
    CASE_STRING(R_ARM_LDRS_SB_G1            )
    CASE_STRING(R_ARM_LDRS_SB_G2            )
    CASE_STRING(R_ARM_LDC_SB_G0             )
    CASE_STRING(R_ARM_LDC_SB_G1             )
    CASE_STRING(R_ARM_LDC_SB_G2             )
    CASE_STRING(R_ARM_MOVW_BREL_NC          )
    CASE_STRING(R_ARM_MOVT_BREL             )
    CASE_STRING(R_ARM_MOVW_BREL             )
    CASE_STRING(R_ARM_THM_MOVW_BREL_NC      )
    CASE_STRING(R_ARM_THM_MOVT_BREL         )
    CASE_STRING(R_ARM_THM_MOVW_BREL         )
    CASE_STRING(R_ARM_TLS_GOTDESC           )
    CASE_STRING(R_ARM_TLS_CALL              )
    CASE_STRING(R_ARM_TLS_DESCSEQ           )
    CASE_STRING(R_ARM_THM_TLS_CALL          )
    CASE_STRING(R_ARM_PLT32_ABS             )
    CASE_STRING(R_ARM_GOT_ABS               )
    CASE_STRING(R_ARM_GOT_PREL              )
    CASE_STRING(R_ARM_GOT_BREL12            )
    CASE_STRING(R_ARM_GOTOFF12              )
    CASE_STRING(R_ARM_GOTRELAX              )
    CASE_STRING(R_ARM_GNU_VTENTRY           )
    CASE_STRING(R_ARM_GNU_VTINHERIT         )
    CASE_STRING(R_ARM_THM_JUMP11            )
    CASE_STRING(R_ARM_THM_JUMP8             )
    CASE_STRING(R_ARM_TLS_GD32              )
    CASE_STRING(R_ARM_TLS_LDM32             )
    CASE_STRING(R_ARM_TLS_LDO32             )
    CASE_STRING(R_ARM_TLS_IE32              )
    CASE_STRING(R_ARM_TLS_LE32              )
    CASE_STRING(R_ARM_TLS_LDO12             )
    CASE_STRING(R_ARM_TLS_LE12              )
    CASE_STRING(R_ARM_TLS_IE12GP            )
    CASE_STRING(R_ARM_PRIVATE_0             )
    CASE_STRING(R_ARM_PRIVATE_1             )
    CASE_STRING(R_ARM_PRIVATE_2             )
    CASE_STRING(R_ARM_PRIVATE_3             )
    CASE_STRING(R_ARM_PRIVATE_4             )
    CASE_STRING(R_ARM_PRIVATE_5             )
    CASE_STRING(R_ARM_PRIVATE_6             )
    CASE_STRING(R_ARM_PRIVATE_7             )
    CASE_STRING(R_ARM_PRIVATE_8             )
    CASE_STRING(R_ARM_PRIVATE_9             )
    CASE_STRING(R_ARM_PRIVATE_10            )
    CASE_STRING(R_ARM_PRIVATE_11            )
    CASE_STRING(R_ARM_PRIVATE_12            )
    CASE_STRING(R_ARM_PRIVATE_13            )
    CASE_STRING(R_ARM_PRIVATE_14            )
    CASE_STRING(R_ARM_PRIVATE_15            )
    CASE_STRING(R_ARM_ME_TOO                )
    CASE_STRING(R_ARM_THM_TLS_DESCSEQ16     )
    CASE_STRING(R_ARM_THM_TLS_DESCSEQ32     )
    CASE_STRING(R_ARM_THM_BF16              )
    CASE_STRING(R_ARM_THM_BF12              )
    CASE_STRING(R_ARM_THM_BF18              )
    case 140: case 141: case 142: case 143: case 144:
    case 145: case 146: case 147: case 148: case 149:
    case 150: case 151: case 152: case 153: case 154:
    case 155: case 156: case 157: case 158: case 159:
        return "Dynamic Reserved";
    CASE_STRING(R_ARM_IRELATIVE             )
    default: return "Unallocated";
    }
}


void
ELF_print_relocations(
    const struct object& obj,
    std::vector<section_reloc> relocations)
{
    if (relocations.size() == 0) return;
    std::cout << "===========================\n";
    std::cout << "==      RELOCATIONS      ==\n";
    std::cout << "===========================\n\n";
    std::cout << "Index | Offset   | Type                     | Symbol   \n";
    std::cout << "-------------------------------------------------------\n";
    for (size_t i = 0; i < relocations.size(); i++) {
        const auto& entry = relocations[i];
        std::cout << std::hex << std::setw(5) << i << "   " << std::left;
        std::cout << std::hex << std::setw(8) << entry.offset << "   ";
        std::cout << std::setw(24) << ELF_relocation_type2str(entry.type) << "   ";
        auto sym_idx = entry.sym_index;
        if (sym_idx < obj.symbol_names.size()) {
            std::cout << obj.symbol_names[sym_idx] << " ";
            std::cout << obj.section_names[obj.symbols[sym_idx].st_shndx] << "@";
            std::cout << "x" << std::hex << obj.symbols[sym_idx].st_value;
        }
        else std::cout << "<unknown>";
        std::cout << "\n" << std::right;
    }
    std::cout << "-------------------------------------------------------\n";
    std::cout << std::dec << std::setfill(' ') << std::right << std::endl;
}


static void
ELF_print_all_relocations(const struct object& obj)
{
    std::cout << "===========================\n";
    std::cout << "==      RELOCATIONS      ==\n";
    std::cout << "===========================\n\n";
    std::cout << "Index | Offset   | Type                     | Symbol   \n";
    std::cout << "-------------------------------------------------------\n";
    for (size_t i = 0; i < obj.relocation_entries.size(); i++) {
        const auto& entry = obj.relocation_entries[i];
        std::cout << std::hex << std::setw(5) << i << "   ";
        std::cout << std::left;
        std::cout << std::hex << std::setw(8) << entry.offset << "   ";
        std::cout << std::setw(24) <<
            ELF_relocation_type2str(ELF32_R_TYPE(entry.info)) << "   ";
        auto symbol = ELF32_R_SYM(entry.info);
        if (symbol < obj.symbol_names.size())
            std::cout << obj.symbol_names[symbol];
        else std::cout << "<unknown>";
        std::cout << "\n" << std::right;
    }
    std::cout << "-------------------------------------------------------\n";
    std::cout << std::dec << std::setfill(' ') << std::right << std::endl;
}


void
ELF_print(const struct object& obj)
{
    std::vector<size_t> symbol_indices(obj.symbols.size());
    std::iota(symbol_indices.begin(), symbol_indices.end(), 0);
    ELF_print_header(obj);
    ELF_print_section_headers(obj);
    ELF_print_symbols(obj, symbol_indices);
    ELF_print_all_relocations(obj);
}



void
ELF_print_section_header(
    const struct object& obj,
    size_t idx)
{
    const auto& hdr = obj.section_headers[idx];
    std::cout << "name:      " << obj.section_names[idx];
    std::cout << "(" << std::dec << hdr.sh_name << ")\n";
    std::cout << "type:      " << ELF_section_header_type2str(hdr.sh_type) << "\n";
    std::cout << "flags:     " << ELF_section_flags2str(hdr.sh_flags) << "\n";
    std::cout << "addr:      0x" << std::hex << hdr.sh_addr      << "\n";
    std::cout << "offset:    0x" << std::hex << hdr.sh_offset    << "\n";
    std::cout << "size:      0x" << std::hex << hdr.sh_size      << "\n";
    std::cout << "link:      0x" << std::hex << hdr.sh_link      << "\n";
    std::cout << "info:      0x" << std::hex << hdr.sh_info      << "\n";
    std::cout << "addralign: 0x" << std::hex << hdr.sh_addralign << "\n";
    std::cout << "entsize:   0x" << std::hex << hdr.sh_entsize   << "\n";
    std::cout << "\n";
}


void
ELF_print_sections(const struct object& obj)
{ ELF_print_section_headers(obj); }


void
ELF_print_detailed_sections(const struct object& obj)
{
    for (const auto& section : obj.sections) {
        std::cout << "====================================================================";
        std::cout << "====================================\n";

        std::cout << "Section: " << section.name << "\n\n";
        hexdump(section.raw_data);
        std::cout << "\n";

        ELF_print_symbols(obj, section.symbol_indices);

        if (section.relocations.size() > 0) {
            std::cout << "===========================\n";
            std::cout << "==      RELOCATIONS      ==\n";
            std::cout << "===========================\n\n";
            std::cout << "Index | Offset   | Type                     | Symbol   \n";
            std::cout << "--------------------------------------------------------------------";
            std::cout << "------------------------------------\n";
            std::cout << std::left;
            for (size_t i = 0; i < section.relocations.size(); i++) {
                const auto& entry = section.relocations[i];
                std::cout << std::hex << std::setw(5) << i << "   ";
                std::cout << std::hex << std::setw(8) << entry.offset << "   ";
                std::cout << std::setw(24) <<
                    ELF_relocation_type2str(entry.type) << "   ";
                if (entry.sym_index < obj.symbol_names.size())
                    std::cout << obj.symbol_names[entry.sym_index];
                else std::cout << "<unknown>";
                std::cout << "\n";
            }
            std::cout << "--------------------------------------------------------------------";
            std::cout << "------------------------------------\n";
            std::cout << std::dec << std::setfill(' ') << std::right;
        }

        std::cout << "\n\n\n";
    }

    //     std::vector<section_reloc> relocations;
}


void
hexdump(const std::string& data)
{
    if (data.size() == 0) return;
    std::cout << std::setfill('0');
    size_t offset = 0;
    do {
        std::cout << "\t" << std::hex << std::setw(4) << offset << " ";
        size_t size = std::min<size_t>(data.size() - offset, 16);
        std::string line(&data[offset], size);
        offset += size;
        for (size_t i = 0; i < 16; i++) {
            if (i % 4 == 0) std::cout << " ";
            if (i < size)
                std::cout << std::hex << std::setw(2) << ((int)line[i] & 0xFF);
            else std::cout << "  ";
        }
        std::cout << "\t";
        for (size_t i = 0; i < size; i++) {
            char c = line[i];
            if (!(c >= ' ' && c <= '~')) c = '.';
            std::cout << c;
        }
        std::cout << "\n";
    } while (offset < data.size());
}
