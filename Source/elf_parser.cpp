/**
 * Created by TekuConcept on February 18, 2020
 */

#include "elf_parser.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <algorithm>

#define swap16(x) x = __builtin_bswap16(x)
#define swap32(x) x = __builtin_bswap32(x)


static void
ELF_parse_header(
    std::istream& file,
    struct object& obj)
{
    file.read((char*)&obj.header, sizeof(obj.header));
    auto& hdr = obj.header;
    assert(obj.header.e_ident[EI_MAG0] == 0x7F);
    assert(obj.header.e_ident[EI_MAG1] == 'E');
    assert(obj.header.e_ident[EI_MAG2] == 'L');
    assert(obj.header.e_ident[EI_MAG3] == 'F');
    assert(obj.header.e_ident[EI_CLASS] == ELFCLASS32);
    assert(obj.header.e_ident[EI_DATA] == ELFDATA2LSB);
    assert(obj.header.e_ident[EI_VERSION] == EV_CURRENT);
}


static void
ELF_read_section_data(
    std::istream& file,
    struct object& obj)
{
    // TODO: do not read virtual sections
    obj.section_rawdata.resize(obj.section_headers.size());
    for (size_t i = 0; i < obj.section_headers.size(); i++) {
        if (obj.section_headers[i].sh_type == SHT_NOBITS) continue;
        auto offset = obj.section_headers[i].sh_offset;
        auto size = obj.section_headers[i].sh_size;
        obj.section_rawdata[i].resize(size, '\0');
        file.seekg(offset, std::ios_base::beg);
        file.read(&obj.section_rawdata[i][0], obj.section_rawdata[i].size());
    }
}


static void
ELF_parse_section_table(
    std::istream& file,
    struct object& obj)
{
    if (obj.header.e_shoff == 0) return; // no section header
    assert(sizeof(Elf32_Shdr) == obj.header.e_shentsize);

    file.seekg(obj.header.e_shoff, std::ios_base::beg);
    obj.section_headers.resize(obj.header.e_shnum);
    file.read((char*)obj.section_headers.data(),
        obj.header.e_shnum * obj.header.e_shentsize);

    ELF_read_section_data(file, obj);

    if (obj.header.e_shstrndx == SHN_UNDEF)
        obj.section_names.resize(obj.section_headers.size());
    else {
        for (size_t i = 0; i < obj.section_headers.size(); i++) {
            auto offset = obj.section_headers[i].sh_name;
            obj.section_names.push_back(std::string(
                &obj.section_rawdata[obj.header.e_shstrndx][offset]));
        }
    }
}


static void
ELF_parse_symbol_table(struct object& obj)
{
    for (size_t i = 0; i < obj.section_headers.size(); i++) {
        if (obj.section_headers[i].sh_type != SHT_SYMTAB) continue;
        auto size_in_bytes = obj.section_headers[i].sh_size;
        auto entry_size = obj.section_headers[i].sh_entsize;
        auto size = size_in_bytes / entry_size;
        auto old_size = obj.symbols.size();
        
        assert(obj.section_rawdata[i].size() == size_in_bytes);
        assert(sizeof(Elf32_Sym) == entry_size);
        obj.symbols.resize(old_size + size);
        memcpy(
            (char*)&obj.symbols[old_size],
            &obj.section_rawdata[i][0],
            size_in_bytes
        );

        auto link = obj.section_headers[i].sh_link;
        if (link == 0)
            obj.symbol_names.resize(obj.symbol_names.size() + size);
        else {
            assert(link < obj.section_headers.size());
            const auto& strtab = obj.section_rawdata[link];
            for (size_t j = 0; j < size; j++)
                obj.symbol_names.push_back(
                    std::string(&strtab[obj.symbols[j].st_name]));
        }
    }
}


static void
ELF_parse_relocations(struct object& obj)
{
    for (size_t i = 0; i < obj.section_headers.size(); i++) {
        if (obj.section_headers[i].sh_type != SHT_REL) continue;
        auto size_in_bytes = obj.section_headers[i].sh_size;
        auto entry_size = obj.section_headers[i].sh_entsize;
        auto size = size_in_bytes / entry_size;
        std::vector<Elf32_Rel> table(size);

        assert(obj.section_rawdata[i].size() == size_in_bytes);
        assert(sizeof(Elf32_Rel) == entry_size);
        memcpy(
            (char*)&table[0],
            &obj.section_rawdata[i][0],
            size_in_bytes
        );

        auto link = obj.section_headers[i].sh_link;
        for (auto entry : table) {
            struct reloc_entry e;
            e.offset    = entry.r_offset;
            e.info      = entry.r_info;
            e.link      = link;
            e.section   = i;
            obj.relocation_entries.push_back(e);
        }
    }
}


static void
ELF_organize_sections(struct object& obj)
{
    std::vector<size_t> tagged_sections;

    for (size_t i = 0; i < obj.symbols.size(); i++) {
        if (ELF32_ST_TYPE(obj.symbols[i].st_info) != STT_SECTION)
            continue;
        tagged_sections.push_back(obj.symbols[i].st_shndx);
    }

    for (auto sidx : tagged_sections) {
        if (sidx >= obj.section_headers.size()) continue;

        struct section_t section;
        memcpy(
            (void*)&section.header,
            (void*)&obj.section_headers[sidx],
            sizeof(Elf32_Shdr)
        );
        section.header_index = sidx;
        section.name = obj.section_names[sidx];
        section.raw_data = obj.section_rawdata[sidx];

        for (size_t i = 0; i < obj.symbols.size(); i++) {
            if (obj.symbols[i].st_shndx == sidx)
                section.symbol_indices.push_back(i);
        }

        for (size_t i = 0; i < obj.section_headers.size(); i++) {
            if (obj.section_headers[i].sh_type != SHT_REL ||
                obj.section_headers[i].sh_info != sidx) continue;
            auto size_in_bytes = obj.section_headers[i].sh_size;
            auto entry_size = obj.section_headers[i].sh_entsize;
            auto size = size_in_bytes / entry_size;
            std::vector<Elf32_Rel> table(size);
            assert(obj.section_rawdata[i].size() == size_in_bytes);
            assert(sizeof(Elf32_Rel) == entry_size);
            memcpy(
                (char*)&table[0],
                &obj.section_rawdata[i][0],
                size_in_bytes
            );
            auto link = obj.section_headers[i].sh_link;
            assert(link < obj.section_rawdata.size());
            for (size_t j = 0; j < table.size(); j++) {
                auto& entry = table[j];
                struct section_reloc e;
                e.offset     = entry.r_offset;
                e.type       = ELF32_R_TYPE(entry.r_info);
                e.sym_index  = ELF32_R_SYM(entry.r_info);
                section.relocations.push_back(e);
            }
        }

        obj.sections.push_back(section);
    }
}


struct object
ELF_parse(std::istream& file)
{
    struct object result;

    ELF_parse_header(file, result);
    ELF_parse_section_table(file, result);
    ELF_parse_symbol_table(result);
    ELF_parse_relocations(result);

    ELF_organize_sections(result);

    return result;
}


void
ELF_sort_section_syms_by_value(
    struct object& obj,
    size_t index)
{
    if (index >= obj.sections.size()) return;
    if (obj.sections[index].symbol_indices.size() <= 1) return;

    std::sort(
        obj.sections[index].symbol_indices.begin(),
        obj.sections[index].symbol_indices.end(),
        [&](const size_t& a, const size_t& b) {
            return (obj.symbols[a].st_value < obj.symbols[b].st_value);
        }
    );
}


void
ELF_sort_section_relocs_by_offset(
    struct object& obj,
    size_t index)
{
    if (index >= obj.sections.size()) return;
    if (obj.sections[index].symbol_indices.size() <= 1) return;

    std::sort(
        obj.sections[index].relocations.begin(),
        obj.sections[index].relocations.end(),
        [&](const section_reloc& a, const section_reloc& b) {
            return a.offset < b.offset;
        }
    );
}
