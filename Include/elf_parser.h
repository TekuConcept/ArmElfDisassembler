#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include "elf.h"

struct reloc_entry {
    Elf32_Addr offset;
    Elf32_Word info;
    Elf32_Word link;
    Elf32_Word section;
};

struct section_reloc {
    Elf32_Addr offset;
    Elf32_Word type;
    size_t sym_index;
};

struct section_t {
    std::string name;
    std::string raw_data;
    std::vector<size_t> symbol_indices;
    std::vector<section_reloc> relocations;
    Elf32_Shdr header;
    size_t header_index;
};

struct object {
    Elf32_Ehdr header;

    std::vector<Elf32_Shdr> section_headers;
    std::vector<std::string> section_names;
    std::vector<std::string> section_rawdata;

    std::vector<Elf32_Sym> symbols;
    std::vector<std::string> symbol_names;
    std::vector<struct reloc_entry> relocation_entries;

    std::vector<struct section_t> sections;
};

struct object ELF_parse(std::istream& file);

void ELF_sort_section_syms_by_value(struct object& obj, size_t index);
void ELF_sort_section_relocs_by_offset(struct object& obj, size_t index);

#endif
