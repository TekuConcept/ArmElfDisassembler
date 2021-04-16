/**
 * Created by TekuConcept on August 4, 2020
 */

#ifndef ELF_OBJECT_H
#define ELF_OBJECT_H

#include <iostream>
#include <vector>
#include <string>

class elf_object {
public:
    enum class section_type_t {
        UNDEFINED          =  0,
        PROGBITS           =  1,
        SYMTAB             =  2,
        STRTAB             =  3,
        RELA               =  4,
        HASH               =  5,
        DYNAMIC            =  6,
        NOTE               =  7,
        NOBITS             =  8,
        REL                =  9,
        SHLIB              = 10,
        DYNSYM             = 11,
        ARM_EXIDX          = 0x70000001,
        ARM_PREEMPTMAP     = 0x70000002,
        ARM_ATTRIBUTES     = 0x70000003,
        ARM_DEBUGOVERLAY   = 0x70000004,
        ARM_OVERLAYSECTION = 0x70000005,
        UNKNOWN
    };
    enum class sym_bind_t { LOCAL, GLOBAL, WEAK, UNKNOWN };
    enum class sym_type_t { NOTYPE, OBJECT, FUNCTION, SECTION, FILE, UNKNOWN };
    enum class reloc_type_t {
        R_ARM_NONE                  =  0,  // Static Misc
        R_ARM_PC24                  =  1,  // Deprecated ARM ((S + A) | T) - P
        R_ARM_ABS32                 =  2,  // Static Data (S + A) | T
        R_ARM_REL32                 =  3,  // Static Data ((S + A) | T) - P
        R_ARM_LDR_PC_G0             =  4,  // Static ARM S + A - P
        R_ARM_ABS16                 =  5,  // Static Data S + A
        R_ARM_ABS12                 =  6,  // Static ARM S + A
        R_ARM_THM_ABS5              =  7,  // Static Thumb16 S + A
        R_ARM_ABS8                  =  8,  // Static Data S + A
        R_ARM_SBREL32               =  9,  // Static Data ((S + A) | T) - B(S)
        R_ARM_THM_CALL              =  10, // Static Thumb32 ((S + A) | T) - P
        R_ARM_THM_PC8               =  11, // Static Thumb16
        R_ARM_BREL_ADJ              =  12, // Dynamic Data DeltaB(S) + A
        R_ARM_TLS_DESC              =  13, // Dynamic Data
        R_ARM_THM_SWI8              =  14, // Obsolete
        R_ARM_XPC25                 =  15, // Obsolete
        R_ARM_THM_XPC22             =  16, // Obsolete
        R_ARM_TLS_DTPMOD32          =  17, // Dynamic Data Module(S)
        R_ARM_TLS_DTPOFF32          =  18, // Dynamic Data S + A - TLS
        R_ARM_TLS_TPOFF32           =  19, // Dynamic Data S + A - tp
        R_ARM_COPY                  =  20, // Dynamic Misc
        R_ARM_GLOB_DAT              =  21, // Dynamic Data (S + A) | T
        R_ARM_JUMP_SLOT             =  22, // Dynamic Data (S + A) | T
        R_ARM_RELATIVE              =  23, // Dynamic Data B(S) + A
        R_ARM_GOTOFF32              =  24, // Static Data (((S + A) | T) - GOT_ORG
        R_ARM_BASE_PREL             =  25, // Static Data B(S) + A - P
        R_ARM_GOT_BREL              =  26, // Static Data GOT(S) + A - GOT_ORG
        R_ARM_PLT32                 =  27, // Deprecated ARM ((S + A) | T) - P
        R_ARM_CALL                  =  28, // Static ARM ((S + A) | T) - P
        R_ARM_JUMP24                =  29, // Static ARM ((S + A) | T) - P
        R_ARM_THM_JUMP24            =  30, // Static Thumb32 ((S + A) | T) - P
        R_ARM_BASE_ABS              =  31, // Static Data B(S) + A
        R_ARM_ALU_PCREL_7_0         =  32, // Obsolete
        R_ARM_ALU_PCREL_15_8        =  33, // Obsolete
        R_ARM_ALU_PCREL_23_15       =  34, // Obsolete
        R_ARM_LDR_SBREL_11_0_NC     =  35, // Deprecated ARM S + A - B(S)
        R_ARM_ALU_SBREL_19_12_NC    =  36, // Deprecated ARM S + A - B(S)
        R_ARM_ALU_SBREL_27_20_CK    =  37, // Deprecated ARM S + A - B(S)
        R_ARM_TARGET1               =  38, // Data  Misc (S + A) | T or //   ((S + A) | T) - P
        R_ARM_SBREL31               =  39, // Deprecated Data ((S + A) | T) - B(S)
        R_ARM_V4BX                  =  40, // Static Misc 
        R_ARM_TARGET2               =  41, // Static Misc
        R_ARM_PREL31                =  42, // Static Data ((S + A) | T) - P
        R_ARM_MOVW_ABS_NC           =  43, // Static ARM (S + A) | T
        R_ARM_MOVT_ABS              =  44, // Static ARM S + A
        R_ARM_MOVW_PREL_NC          =  45, // Static ARM ((S + A) | T) - P
        R_ARM_MOVT_PREL             =  46, // Static ARM S + A - P
        R_ARM_THM_MOVW_ABS_NC       =  47, // Static Thumb32 (S + A) | T
        R_ARM_THM_MOVT_ABS          =  48, // Static Thumb32 S + A - P
        R_ARM_THM_MOVW_PREL_NC      =  49, // Static Thumb32 ((S + A) | T) - P
        R_ARM_THM_MOVT_PREL         =  50, // Static Thumb32 S + A - P
        R_ARM_THM_JUMP19            =  51, // Static Thumb32 ((S + A) | T) - P
        R_ARM_THM_JUMP6             =  52, // Static Thumb16 S + A - P
        R_ARM_THM_ALU_PREL_11_0     =  53, // Static Thumb32 ((S + A) | T) - Pa
        R_ARM_THM_PC12              =  54, // Static Thumb32 S + A - Pa
        R_ARM_ABS32_NOI             =  55, // Static Data S + A
        R_ARM_REL32_NOI             =  56, // Static Data S + A - P
        R_ARM_ALU_PC_G0_NC          =  57, // Static ARM ((S + A) | T) - P
        R_ARM_ALU_PC_G0             =  58, // Static ARM ((S + A) | T) - P
        R_ARM_ALU_PC_G1_NC          =  59, // Static ARM ((S + A) | T) - P
        R_ARM_ALU_PC_G1             =  60, // Static ARM ((S + A) | T) - P
        R_ARM_ALU_PC_G2             =  61, // Static ARM ((S + A) | T) - P
        R_ARM_LDR_PC_G1             =  62, // Static ARM S + A - P
        R_ARM_LDR_PC_G2             =  63, // Static ARM S + A - P
        R_ARM_LDRS_PC_G0            =  64, // Static ARM S + A - P
        R_ARM_LDRS_PC_G1            =  65, // Static ARM S + A - P
        R_ARM_LDRS_PC_G2            =  66, // Static ARM S + A - P
        R_ARM_LDC_PC_G0             =  67, // Static ARM S + A - P
        R_ARM_LDC_PC_G1             =  68, // Static ARM S + A - P
        R_ARM_LDC_PC_G2             =  69, // Static ARM S + A - P
        R_ARM_ALU_SB_G0_NC          =  70, // Static ARM ((S + A) | T) - B(S)
        R_ARM_ALU_SB_G0             =  71, // Static ARM ((S + A) | T) - B(S)
        R_ARM_ALU_SB_G1_NC          =  72, // Static ARM ((S + A) | T) - B(S)
        R_ARM_ALU_SB_G1             =  73, // Static ARM ((S + A) | T) - B(S)
        R_ARM_ALU_SB_G2             =  74, // Static ARM ((S + A) | T) - B(S)
        R_ARM_LDR_SB_G0             =  75, // Static ARM S + A - B(S)
        R_ARM_LDR_SB_G1             =  76, // Static ARM S + A - B(S)
        R_ARM_LDR_SB_G2             =  77, // Static ARM S + A - B(S)
        R_ARM_LDRS_SB_G0            =  78, // Static ARM S + A - B(S)
        R_ARM_LDRS_SB_G1            =  79, // Static ARM S + A - B(S)
        R_ARM_LDRS_SB_G2            =  80, // Static ARM S + A - B(S)
        R_ARM_LDC_SB_G0             =  81, // Static ARM S + A - B(S)
        R_ARM_LDC_SB_G1             =  82, // Static ARM S + A - B(S)
        R_ARM_LDC_SB_G2             =  83, // Static ARM S + A - B(S)
        R_ARM_MOVW_BREL_NC          =  84, // Static ARM ((S + A) | T) - B(S)
        R_ARM_MOVT_BREL             =  85, // Static ARM S + A - B(S)
        R_ARM_MOVW_BREL             =  86, // Static ARM ((S + A) | T) - B(S)
        R_ARM_THM_MOVW_BREL_NC      =  87, // Static Thumb32 ((S + A) | T) - B(S)
        R_ARM_THM_MOVT_BREL         =  88, // Static Thumb32 S + A - B(S)
        R_ARM_THM_MOVW_BREL         =  89, // Static Thumb32 ((S + A) | T) - B(S)
        R_ARM_TLS_GOTDESC           =  90, // Static Data
        R_ARM_TLS_CALL              =  91, // Static ARM
        R_ARM_TLS_DESCSEQ           =  92, // Static ARM TLS relaxation
        R_ARM_THM_TLS_CALL          =  93, // Static Thumb32
        R_ARM_PLT32_ABS             =  94, // Static Data PLT(S) + A
        R_ARM_GOT_ABS               =  95, // Static Data GOT(S) + A
        R_ARM_GOT_PREL              =  96, // Static Data GOT(S) + A - P
        R_ARM_GOT_BREL12            =  97, // Static ARM GOT(S) + A - GOT_ORG
        R_ARM_GOTOFF12              =  98, // Static ARM S + A - GOT_ROG
        R_ARM_GOTRELAX              =  99, // Static Misc
        R_ARM_GNU_VTENTRY           = 100, // Deprecated Data
        R_ARM_GNU_VTINHERIT         = 101, // Deprecated Data
        R_ARM_THM_JUMP11            = 102, // Static Thumb16 S + A - P
        R_ARM_THM_JUMP8             = 103, // Static Thumb16 S + A - P
        R_ARM_TLS_GD32              = 104, // Static Data GOT(S) + A - P
        R_ARM_TLS_LDM32             = 105, // Static Data GOT(S) + A - P
        R_ARM_TLS_LDO32             = 106, // Static Data S + A - TLS
        R_ARM_TLS_IE32              = 107, // Static Data GOT(S) + A - P
        R_ARM_TLS_LE32              = 108, // Static Data S + A - tp
        R_ARM_TLS_LDO12             = 109, // Static ARM S + A - TLS
        R_ARM_TLS_LE12              = 110, // Static ARM S + A - tp
        R_ARM_TLS_IE12GP            = 111, // Static ARM GOT(S) + A - GOT_ORG
        R_ARM_PRIVATE_0             = 112, // Private (n = 0, 1, ... 15)
        R_ARM_PRIVATE_1             = 113,
        R_ARM_PRIVATE_2             = 114,
        R_ARM_PRIVATE_3             = 115,
        R_ARM_PRIVATE_4             = 116,
        R_ARM_PRIVATE_5             = 117,
        R_ARM_PRIVATE_6             = 118,
        R_ARM_PRIVATE_7             = 119,
        R_ARM_PRIVATE_8             = 120,
        R_ARM_PRIVATE_9             = 121,
        R_ARM_PRIVATE_10            = 122,
        R_ARM_PRIVATE_11            = 123,
        R_ARM_PRIVATE_12            = 124,
        R_ARM_PRIVATE_13            = 125,
        R_ARM_PRIVATE_14            = 126,
        R_ARM_PRIVATE_15            = 127,
        R_ARM_ME_TOO                = 128, // Obsolete
        R_ARM_THM_TLS_DESCSEQ16     = 129, // Static Thumb16
        R_ARM_THM_TLS_DESCSEQ32     = 130, // Static Thumb32
        R_ARM_THM_BF16              = 136, // Static       Thumb32 ((S + A) | T) – P
        R_ARM_THM_BF12              = 137, // Static       Thumb32 ((S + A) | T) – P
        R_ARM_THM_BF18              = 138, // Static       Thumb32 ((S + A) | T) – P
        R_ARM_IRELATIVE             = 160, // Dynamic
        DYNAMIC_RESERVED, // 140 - 159
        UNALLOCATED
    };

    struct header_info_t {
        unsigned int offset;
        unsigned int entry_size;
        unsigned int entry_count;
    };

    struct section_t;
    struct symbol_t {
        std::string name;
        unsigned int value;
        unsigned int size;
        sym_bind_t bind;
        sym_type_t type;
        unsigned char other;
        section_t* section;
    };

    struct relocation_t {
        unsigned int offset;
        reloc_type_t type;
        symbol_t* symbol;
        section_t* section;
        unsigned int sidx;
    };

    struct section_t {
        std::string name;
        std::string raw_data;
        section_type_t type;
        section_t* link;
        std::vector<symbol_t*> symbols;
        unsigned int flags;
        unsigned int address;
        unsigned int offset;
        unsigned int size;
        unsigned int info;
        unsigned int align;
        unsigned int entry_size;
        
        std::vector<size_t> entry_indices;
    };

    static elf_object parse(std::istream& stream);
    static std::string section_type_string(section_type_t type);
    static std::string reloc_type_string(reloc_type_t type);
    static std::string section_flags_string(unsigned int flags);

    elf_object() = default;

    inline const std::vector<section_t>& sections() const
    { return m_sections; }

    inline const std::vector<symbol_t>& symbols() const
    { return m_symbols; }

    inline const std::vector<relocation_t>& relocations() const
    { return m_relocations; }

    inline const std::string& id() const { return m_id; }
    inline const std::string& type() const { return m_type; }
    inline const std::string& machine() const { return m_machine; }
    inline unsigned int version() const { return m_version; }
    inline unsigned int entry() const { return m_entry; }
    inline unsigned int flags() const { return m_flags; }
    inline unsigned int elf_header_size() const { return m_elf_header_size; }
    inline unsigned int string_table_index() const { return m_string_table_index; }
    inline const header_info_t& program_header_info() const { return m_program_header_info; }
    inline const header_info_t& section_header_info() const { return m_section_header_info; }

private:
    std::string m_id;
    std::string m_type;
    std::string m_machine;
    unsigned int m_version;
    unsigned int m_entry;
    unsigned int m_flags;
    unsigned int m_elf_header_size;
    unsigned int m_string_table_index;
    header_info_t m_program_header_info;
    header_info_t m_section_header_info;

    std::vector<section_t> m_sections;
    std::vector<symbol_t> m_symbols;
    std::vector<relocation_t> m_relocations;
};

#endif
