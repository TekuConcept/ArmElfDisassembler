#ifndef ELF_PRINTER_H
#define ELF_PRINTER_H

#include <string>
#include <vector>
#include "elf_parser.h"

void ELF_print(const struct object& obj);
void ELF_print_symbols(const struct object& obj, const std::vector<size_t>& indices);
void ELF_print_relocations(const struct object& obj, std::vector<section_reloc> relocations);
void ELF_print_section_header(const struct object& obj, size_t idx);
void ELF_print_sections(const struct object& obj);
void ELF_print_detailed_sections(const struct object& obj);
void hexdump(const std::string& data);

#endif
