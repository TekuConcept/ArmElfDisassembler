/**
 * Created by TekuConcept on August 4, 2020
 */

#include "elf_object.h"
#include <cstring>
#include <stdexcept>
#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "elf.h"

elf_object
elf_object::parse(std::istream& __stream)
{
    elf_object obj;

    Elf32_Ehdr header;
    std::vector<Elf32_Shdr> section_headers;
    std::map<size_t,size_t> symbol_link_map;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // READ HEADER
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    __stream.read((char*)&header, sizeof(header));
    if ((header.e_ident[EI_MAG0]    != 0x7F)        ||
        (header.e_ident[EI_MAG1]    != 'E')         ||
        (header.e_ident[EI_MAG2]    != 'L')         ||
        (header.e_ident[EI_MAG3]    != 'F'))
        throw std::runtime_error("Not an ELF file");
    else if (header.e_ident[EI_CLASS]   != ELFCLASS32)
        throw std::runtime_error("Only 32-bit ELF supported");
    else if (header.e_ident[EI_DATA]    != ELFDATA2LSB)
        throw std::runtime_error("Only ELFDATA2LSB supported");
    else if (header.e_ident[EI_VERSION] != EV_CURRENT)
        throw std::runtime_error("Unsupported ELF version");

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PARSE HEADER
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    {
        std::ostringstream os;
        for (size_t i = 0; i < EI_NIDENT; i++)
            os << std::hex << std::setw(2) << (int)header.e_ident[i];
        obj.m_id = os.str();

        switch (header.e_type) {
        case ET_NONE: obj.m_type = "None";          break;
        case ET_REL:  obj.m_type = "Relocatable";   break;
        case ET_EXEC: obj.m_type = "Executable";    break;
        case ET_DYN:  obj.m_type = "Shared Object"; break;
        case ET_CORE: obj.m_type = "Core";          break;
        default:
            os.str("");
            os << "Unknown (0x" << std::hex << header.e_type << ")";
            obj.m_type = os.str();
            break;
        }

        switch (header.e_machine) {
        case EM_NONE:  obj.m_machine = "None";  break;
        case EM_M32:   obj.m_machine = "M32";   break;
        case EM_SPARC: obj.m_machine = "SPARC"; break;
        case EM_386:   obj.m_machine = "386";   break;
        case EM_68K:   obj.m_machine = "68K";   break;
        case EM_88K:   obj.m_machine = "88K";   break;
        case EM_860:   obj.m_machine = "860";   break;
        case EM_MIPS:  obj.m_machine = "MIPS";  break;
        case EM_ARM:   obj.m_machine = "ARM";   break;
        default:
            os.str("");
            os << "Unknown (0x" << std::hex << header.e_machine << ")";
            obj.m_machine = os.str();
            break;
        }
    }

    obj.m_version            = header.e_version;
    obj.m_entry              = header.e_entry;
    obj.m_flags              = header.e_flags;
    obj.m_elf_header_size    = header.e_ehsize;
    obj.m_string_table_index = header.e_shstrndx;

    obj.m_program_header_info.offset      = header.e_phoff;
    obj.m_program_header_info.entry_size  = header.e_phentsize;
    obj.m_program_header_info.entry_count = header.e_phnum;

    obj.m_section_header_info.offset      = header.e_shoff;
    obj.m_section_header_info.entry_size  = header.e_shentsize;
    obj.m_section_header_info.entry_count = header.e_shnum;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // READ SECTION TABLE
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (header.e_shoff == 0 /*no section header*/) return obj;
    if (sizeof(Elf32_Shdr) != header.e_shentsize)
        throw std::runtime_error("section header entry size incorrect");

    __stream.seekg(header.e_shoff, std::ios_base::beg);
    section_headers.resize(header.e_shnum);
    __stream.read(
        (char*)section_headers.data(),
        header.e_shnum * header.e_shentsize);

    obj.m_sections.resize(section_headers.size());

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // READ SECTION DATA
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // TODO: do not read virtual sections
    for (size_t i = 0; i < section_headers.size(); i++) {
        if (section_headers[i].sh_type == SHT_NOBITS) continue;
        auto offset = section_headers[i].sh_offset;
        auto size   = section_headers[i].sh_size;
        obj.m_sections[i].raw_data.resize(size, '\0');
        __stream.seekg(offset, std::ios_base::beg);
        __stream.read(
            (char*)obj.m_sections[i].raw_data.data(),
            obj.m_sections[i].raw_data.size()
        );
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PARSE SECTION HEADERS
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (header.e_shstrndx != SHN_UNDEF) {
        if (header.e_shstrndx >= obj.m_sections.size())
            throw std::runtime_error("section header index out of bounds");
        const auto& name_data = obj.m_sections[header.e_shstrndx].raw_data;
        for (size_t i = 0; i < section_headers.size(); i++) {
            auto offset = section_headers[i].sh_name;
            if (offset >= name_data.size()) {
                std::cout << offset << std::endl;
                throw std::runtime_error("header name index out of bounds");
            }
            obj.m_sections[i].name = std::string(&name_data[offset]);
        }
    }

    for (size_t i = 0; i < section_headers.size(); i++) {
        obj.m_sections[i].type       = (section_type_t)section_headers[i].sh_type;
        obj.m_sections[i].flags      = section_headers[i].sh_flags;
        obj.m_sections[i].address    = section_headers[i].sh_addr;
        obj.m_sections[i].offset     = section_headers[i].sh_offset;
        obj.m_sections[i].size       = section_headers[i].sh_size;
        obj.m_sections[i].info       = section_headers[i].sh_info;
        obj.m_sections[i].align      = section_headers[i].sh_addralign;
        obj.m_sections[i].entry_size = section_headers[i].sh_entsize;
        if (section_headers[i].sh_link >= section_headers.size())
            throw std::runtime_error("section link out of bounds");
        obj.m_sections[i].link = &obj.m_sections[section_headers[i].sh_link];
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PARSE SYMBOL TABLE
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (size_t i = 0; i < section_headers.size(); i++) {
        if ((section_headers[i].sh_type != SHT_SYMTAB) &&
            (section_headers[i].sh_type != SHT_DYNSYM)) continue;

        auto size_in_bytes = section_headers[i].sh_size;
        auto entry_size    = section_headers[i].sh_entsize;
        auto size          = size_in_bytes / entry_size;
        
        if (size == 0) continue;
        if ((obj.m_sections[i].raw_data.size() != size_in_bytes) ||
            (sizeof(Elf32_Sym) != entry_size))
            throw std::runtime_error("symbol data size incorrect");

        symbol_link_map[i] = obj.m_symbols.size();

        std::vector<Elf32_Sym> symbols(size);
        memcpy(
            (char*)&symbols[0],
            &obj.m_sections[i].raw_data[0],
            size_in_bytes
        );

        auto link = section_headers[i].sh_link;
        if (link >= section_headers.size())
            throw std::runtime_error("symbol link out of bounds");

        for (size_t i = 0; i < size; i++) {
            symbol_t symbol;
            symbol.name = (link == 0) ? "[no sym name]" :
                std::string(&obj.m_sections[link].raw_data[symbols[i].st_name]);
            symbol.value = symbols[i].st_value;
            symbol.size  = symbols[i].st_size;
            symbol.other = symbols[i].st_other;
            switch (ELF32_ST_BIND(symbols[i].st_info)) {
            case STB_LOCAL:  symbol.bind = sym_bind_t::LOCAL;   break;
            case STB_GLOBAL: symbol.bind = sym_bind_t::GLOBAL;  break;
            case STB_WEAK:   symbol.bind = sym_bind_t::WEAK;    break;
            default:         symbol.bind = sym_bind_t::UNKNOWN; break;
            }
            switch (ELF32_ST_TYPE(symbols[i].st_info)) {
            case STT_NOTYPE:  symbol.type = sym_type_t::NOTYPE;   break;
            case STT_OBJECT:  symbol.type = sym_type_t::OBJECT;   break;
            case STT_FUNC:    symbol.type = sym_type_t::FUNCTION; break;
            case STT_SECTION: symbol.type = sym_type_t::SECTION;  break;
            case STT_FILE:    symbol.type = sym_type_t::FILE;     break;
            default:          symbol.type = sym_type_t::UNKNOWN;  break;
            }
            if (symbols[i].st_shndx >= obj.m_sections.size()) symbol.section = nullptr;
            else symbol.section = &obj.m_sections[symbols[i].st_shndx];
            obj.m_symbols.push_back(symbol);
        }
    }

    for (auto& symbol : obj.m_symbols)
        if (symbol.section != nullptr)
            symbol.section->symbols.push_back(&symbol);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // PARSE RELOCATIONS
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (size_t i = 0; i < section_headers.size(); i++) {
        if (section_headers[i].sh_type != SHT_REL) continue;

        auto size_in_bytes = section_headers[i].sh_size;
        auto entry_size = section_headers[i].sh_entsize;
        auto size = size_in_bytes / entry_size;

        if (size == 0) continue;
        if ((obj.m_sections[i].raw_data.size() != size_in_bytes) ||
            (sizeof(Elf32_Rel) != entry_size))
            throw std::runtime_error("relocation data size incorrect");

        std::vector<Elf32_Rel> table(size);
        memcpy(
            (char*)&table[0],
            &obj.m_sections[i].raw_data[0],
            size_in_bytes
        );

        auto link = section_headers[i].sh_link;
        auto symkey = symbol_link_map.find(link);
        if (symkey == symbol_link_map.end())
            throw std::runtime_error("symbol relocation table does not exist");

        for (auto entry : table) {
            relocation_t relocation;

            auto type_id      = ELF32_R_TYPE(entry.r_info);
            auto symbol_index = ELF32_R_SYM(entry.r_info) + symkey->second;

            relocation.offset = entry.r_offset;
            if ((type_id >=   0 && type_id <= 130) ||
                (type_id >= 136 && type_id <= 138) ||
                type_id == 160)
                relocation.type = (reloc_type_t)type_id;
            else if (type_id >= 140 && type_id <= 159)
                relocation.type = reloc_type_t::DYNAMIC_RESERVED;
            else relocation.type = reloc_type_t::UNALLOCATED;
            if (symbol_index > obj.m_symbols.size())
                throw std::runtime_error("symbol index out of bounds");
            else relocation.symbol = &obj.m_symbols[symbol_index];
            if (section_headers[i].sh_info >= section_headers.size())
                relocation.section = nullptr;
            else relocation.section = &obj.m_sections[section_headers[i].sh_info];
            relocation.sidx = section_headers[i].sh_info;

            obj.m_sections[i].entry_indices.push_back(obj.m_relocations.size());
            obj.m_relocations.push_back(relocation);
        }
    }

    return obj;
}


std::string
elf_object::section_type_string(section_type_t type)
{
    switch (type) {
    case section_type_t::UNDEFINED:          return "Null";
    case section_type_t::PROGBITS:           return "Program Bits";
    case section_type_t::SYMTAB:             return "Symbol Table";
    case section_type_t::STRTAB:             return "String Table";
    case section_type_t::RELA:               return "Relocation Entries (w/addends)";
    case section_type_t::HASH:               return "Symbol Hash Table";
    case section_type_t::DYNAMIC:            return "Dynamic Linking";
    case section_type_t::NOTE:               return "Note";
    case section_type_t::NOBITS:             return "No-Bits";
    case section_type_t::REL:                return "Relocation Entries";
    case section_type_t::SHLIB:              return "<reserved>";
    case section_type_t::DYNSYM:             return "Dynamic Symbols";
    case section_type_t::ARM_EXIDX:          return "Arm Exception Index Table";
    case section_type_t::ARM_PREEMPTMAP:     return "BPABI DLL dynamic linking pre-emption map";
    case section_type_t::ARM_ATTRIBUTES:     return "Arm Compatibility attributes";
    case section_type_t::ARM_DEBUGOVERLAY:   return "Arm Debug Overlay";
    case section_type_t::ARM_OVERLAYSECTION: return "Arm Overlay Section";
    default: {
        std::ostringstream os;
        os << "Unknown (0x" << std::hex << (int)type << ")";
        return os.str();
    } break;
    }
}


std::string
elf_object::reloc_type_string(reloc_type_t type)
{
    #define CASE_STRING(enum_class, label) \
        case enum_class::label: return std::string(#label);
    switch (type) {
    CASE_STRING(reloc_type_t, R_ARM_NONE                  )
    CASE_STRING(reloc_type_t, R_ARM_PC24                  )
    CASE_STRING(reloc_type_t, R_ARM_ABS32                 )
    CASE_STRING(reloc_type_t, R_ARM_REL32                 )
    CASE_STRING(reloc_type_t, R_ARM_LDR_PC_G0             )
    CASE_STRING(reloc_type_t, R_ARM_ABS16                 )
    CASE_STRING(reloc_type_t, R_ARM_ABS12                 )
    CASE_STRING(reloc_type_t, R_ARM_THM_ABS5              )
    CASE_STRING(reloc_type_t, R_ARM_ABS8                  )
    CASE_STRING(reloc_type_t, R_ARM_SBREL32               )
    CASE_STRING(reloc_type_t, R_ARM_THM_CALL              )
    CASE_STRING(reloc_type_t, R_ARM_THM_PC8               )
    CASE_STRING(reloc_type_t, R_ARM_BREL_ADJ              )
    CASE_STRING(reloc_type_t, R_ARM_TLS_DESC              )
    CASE_STRING(reloc_type_t, R_ARM_THM_SWI8              )
    CASE_STRING(reloc_type_t, R_ARM_XPC25                 )
    CASE_STRING(reloc_type_t, R_ARM_THM_XPC22             )
    CASE_STRING(reloc_type_t, R_ARM_TLS_DTPMOD32          )
    CASE_STRING(reloc_type_t, R_ARM_TLS_DTPOFF32          )
    CASE_STRING(reloc_type_t, R_ARM_TLS_TPOFF32           )
    CASE_STRING(reloc_type_t, R_ARM_COPY                  )
    CASE_STRING(reloc_type_t, R_ARM_GLOB_DAT              )
    CASE_STRING(reloc_type_t, R_ARM_JUMP_SLOT             )
    CASE_STRING(reloc_type_t, R_ARM_RELATIVE              )
    CASE_STRING(reloc_type_t, R_ARM_GOTOFF32              )
    CASE_STRING(reloc_type_t, R_ARM_BASE_PREL             )
    CASE_STRING(reloc_type_t, R_ARM_GOT_BREL              )
    CASE_STRING(reloc_type_t, R_ARM_PLT32                 )
    CASE_STRING(reloc_type_t, R_ARM_CALL                  )
    CASE_STRING(reloc_type_t, R_ARM_JUMP24                )
    CASE_STRING(reloc_type_t, R_ARM_THM_JUMP24            )
    CASE_STRING(reloc_type_t, R_ARM_BASE_ABS              )
    CASE_STRING(reloc_type_t, R_ARM_ALU_PCREL_7_0         )
    CASE_STRING(reloc_type_t, R_ARM_ALU_PCREL_15_8        )
    CASE_STRING(reloc_type_t, R_ARM_ALU_PCREL_23_15       )
    CASE_STRING(reloc_type_t, R_ARM_LDR_SBREL_11_0_NC     )
    CASE_STRING(reloc_type_t, R_ARM_ALU_SBREL_19_12_NC    )
    CASE_STRING(reloc_type_t, R_ARM_ALU_SBREL_27_20_CK    )
    CASE_STRING(reloc_type_t, R_ARM_TARGET1               )
    CASE_STRING(reloc_type_t, R_ARM_SBREL31               )
    CASE_STRING(reloc_type_t, R_ARM_V4BX                  )
    CASE_STRING(reloc_type_t, R_ARM_TARGET2               )
    CASE_STRING(reloc_type_t, R_ARM_PREL31                )
    CASE_STRING(reloc_type_t, R_ARM_MOVW_ABS_NC           )
    CASE_STRING(reloc_type_t, R_ARM_MOVT_ABS              )
    CASE_STRING(reloc_type_t, R_ARM_MOVW_PREL_NC          )
    CASE_STRING(reloc_type_t, R_ARM_MOVT_PREL             )
    CASE_STRING(reloc_type_t, R_ARM_THM_MOVW_ABS_NC       )
    CASE_STRING(reloc_type_t, R_ARM_THM_MOVT_ABS          )
    CASE_STRING(reloc_type_t, R_ARM_THM_MOVW_PREL_NC      )
    CASE_STRING(reloc_type_t, R_ARM_THM_MOVT_PREL         )
    CASE_STRING(reloc_type_t, R_ARM_THM_JUMP19            )
    CASE_STRING(reloc_type_t, R_ARM_THM_JUMP6             )
    CASE_STRING(reloc_type_t, R_ARM_THM_ALU_PREL_11_0     )
    CASE_STRING(reloc_type_t, R_ARM_THM_PC12              )
    CASE_STRING(reloc_type_t, R_ARM_ABS32_NOI             )
    CASE_STRING(reloc_type_t, R_ARM_REL32_NOI             )
    CASE_STRING(reloc_type_t, R_ARM_ALU_PC_G0_NC          )
    CASE_STRING(reloc_type_t, R_ARM_ALU_PC_G0             )
    CASE_STRING(reloc_type_t, R_ARM_ALU_PC_G1_NC          )
    CASE_STRING(reloc_type_t, R_ARM_ALU_PC_G1             )
    CASE_STRING(reloc_type_t, R_ARM_ALU_PC_G2             )
    CASE_STRING(reloc_type_t, R_ARM_LDR_PC_G1             )
    CASE_STRING(reloc_type_t, R_ARM_LDR_PC_G2             )
    CASE_STRING(reloc_type_t, R_ARM_LDRS_PC_G0            )
    CASE_STRING(reloc_type_t, R_ARM_LDRS_PC_G1            )
    CASE_STRING(reloc_type_t, R_ARM_LDRS_PC_G2            )
    CASE_STRING(reloc_type_t, R_ARM_LDC_PC_G0             )
    CASE_STRING(reloc_type_t, R_ARM_LDC_PC_G1             )
    CASE_STRING(reloc_type_t, R_ARM_LDC_PC_G2             )
    CASE_STRING(reloc_type_t, R_ARM_ALU_SB_G0_NC          )
    CASE_STRING(reloc_type_t, R_ARM_ALU_SB_G0             )
    CASE_STRING(reloc_type_t, R_ARM_ALU_SB_G1_NC          )
    CASE_STRING(reloc_type_t, R_ARM_ALU_SB_G1             )
    CASE_STRING(reloc_type_t, R_ARM_ALU_SB_G2             )
    CASE_STRING(reloc_type_t, R_ARM_LDR_SB_G0             )
    CASE_STRING(reloc_type_t, R_ARM_LDR_SB_G1             )
    CASE_STRING(reloc_type_t, R_ARM_LDR_SB_G2             )
    CASE_STRING(reloc_type_t, R_ARM_LDRS_SB_G0            )
    CASE_STRING(reloc_type_t, R_ARM_LDRS_SB_G1            )
    CASE_STRING(reloc_type_t, R_ARM_LDRS_SB_G2            )
    CASE_STRING(reloc_type_t, R_ARM_LDC_SB_G0             )
    CASE_STRING(reloc_type_t, R_ARM_LDC_SB_G1             )
    CASE_STRING(reloc_type_t, R_ARM_LDC_SB_G2             )
    CASE_STRING(reloc_type_t, R_ARM_MOVW_BREL_NC          )
    CASE_STRING(reloc_type_t, R_ARM_MOVT_BREL             )
    CASE_STRING(reloc_type_t, R_ARM_MOVW_BREL             )
    CASE_STRING(reloc_type_t, R_ARM_THM_MOVW_BREL_NC      )
    CASE_STRING(reloc_type_t, R_ARM_THM_MOVT_BREL         )
    CASE_STRING(reloc_type_t, R_ARM_THM_MOVW_BREL         )
    CASE_STRING(reloc_type_t, R_ARM_TLS_GOTDESC           )
    CASE_STRING(reloc_type_t, R_ARM_TLS_CALL              )
    CASE_STRING(reloc_type_t, R_ARM_TLS_DESCSEQ           )
    CASE_STRING(reloc_type_t, R_ARM_THM_TLS_CALL          )
    CASE_STRING(reloc_type_t, R_ARM_PLT32_ABS             )
    CASE_STRING(reloc_type_t, R_ARM_GOT_ABS               )
    CASE_STRING(reloc_type_t, R_ARM_GOT_PREL              )
    CASE_STRING(reloc_type_t, R_ARM_GOT_BREL12            )
    CASE_STRING(reloc_type_t, R_ARM_GOTOFF12              )
    CASE_STRING(reloc_type_t, R_ARM_GOTRELAX              )
    CASE_STRING(reloc_type_t, R_ARM_GNU_VTENTRY           )
    CASE_STRING(reloc_type_t, R_ARM_GNU_VTINHERIT         )
    CASE_STRING(reloc_type_t, R_ARM_THM_JUMP11            )
    CASE_STRING(reloc_type_t, R_ARM_THM_JUMP8             )
    CASE_STRING(reloc_type_t, R_ARM_TLS_GD32              )
    CASE_STRING(reloc_type_t, R_ARM_TLS_LDM32             )
    CASE_STRING(reloc_type_t, R_ARM_TLS_LDO32             )
    CASE_STRING(reloc_type_t, R_ARM_TLS_IE32              )
    CASE_STRING(reloc_type_t, R_ARM_TLS_LE32              )
    CASE_STRING(reloc_type_t, R_ARM_TLS_LDO12             )
    CASE_STRING(reloc_type_t, R_ARM_TLS_LE12              )
    CASE_STRING(reloc_type_t, R_ARM_TLS_IE12GP            )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_0             )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_1             )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_2             )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_3             )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_4             )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_5             )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_6             )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_7             )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_8             )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_9             )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_10            )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_11            )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_12            )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_13            )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_14            )
    CASE_STRING(reloc_type_t, R_ARM_PRIVATE_15            )
    CASE_STRING(reloc_type_t, R_ARM_ME_TOO                )
    CASE_STRING(reloc_type_t, R_ARM_THM_TLS_DESCSEQ16     )
    CASE_STRING(reloc_type_t, R_ARM_THM_TLS_DESCSEQ32     )
    CASE_STRING(reloc_type_t, R_ARM_THM_BF16              )
    CASE_STRING(reloc_type_t, R_ARM_THM_BF12              )
    CASE_STRING(reloc_type_t, R_ARM_THM_BF18              )
    CASE_STRING(reloc_type_t, DYNAMIC_RESERVED            )
    CASE_STRING(reloc_type_t, R_ARM_IRELATIVE             )
    default: return "Unallocated";
    }
    #undef CASE_STRING
}


std::string
elf_object::section_flags_string(unsigned int flags)
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
