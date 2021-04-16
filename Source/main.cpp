#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>
#include <sstream>
#include <iterator>
#include "elf_parser.h"
#include "elf_printer.h"
#include "arm_disassembler.h"
#include "arm_decompiler.h"

struct funcsym {
    unsigned int start;
    unsigned int size;
    std::string name;
    bool ispublic;
};


static std::vector<funcsym>
filter_function_symbols(
    const struct object& obj,
    size_t sidx)
{
    const auto& section = obj.sections[sidx];
    std::vector<funcsym> functions;
    for (size_t j = 0; j < section.symbol_indices.size(); j++) {
        const auto& symbol = obj.symbols[section.symbol_indices[j]];
        const auto& name = obj.symbol_names[section.symbol_indices[j]];
        if (ELF32_ST_TYPE(symbol.st_info) != STT_FUNC) continue;
        
        auto f = std::find_if(functions.begin(), functions.end(),
        [&](const funcsym& sym) { return sym.start == symbol.st_value; });
        if (f != functions.end()) {
            if (ELF32_ST_BIND(symbol.st_info) == STB_GLOBAL) {
                f->name = name;
                f->ispublic = true;
                assert(f->size == symbol.st_size);
            }
        }
        else {
            funcsym func;
            func.name = name;
            func.ispublic = ELF32_ST_BIND(symbol.st_info) == STB_GLOBAL;
            func.start = symbol.st_value;
            func.size = symbol.st_size;
            functions.push_back(func);
        }
    }
    return functions;
}


static void
reformat_strings(std::vector<std::string>& array)
{
    for (auto& str : array) {
        for (size_t i = 0; i < str.size(); i++)
            if (str[i] == '\t') str[i] = ' ';
        std::istringstream iss(str);
        std::vector<std::string> tokens(
            std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>());
        if (tokens.size() == 0) continue;
        std::ostringstream oss;
        oss << std::setw(12) << std::left << tokens.front() << std::right;
        for (size_t i = 1; i < tokens.size(); i++) {
            if (tokens[i] == ";") {
                auto size = oss.str().size();
                size = 42 - size;
                oss << std::setw(size) << "@";
            }
            else oss << " " << tokens[i];
        }
        str = oss.str();

        auto k = str.find_last_of("@");
        std::string::iterator end;
        if (k != std::string::npos)
            end = str.begin() + k;
        else end = str.end();
        std::transform(str.begin(), end, str.begin(), 
            [](unsigned char c) { return std::toupper(c); });
    }
}


static void
relocate_syms(
    std::vector<std::string>& array,
    struct object& obj,
    size_t sec_idx)
{
    const auto& section = obj.sections[sec_idx];
    ELF_sort_section_relocs_by_offset(obj, sec_idx);
    std::ostringstream os;
    for (const auto& r : section.relocations) {
        size_t aidx = r.offset / 4;
        assert(aidx < array.size());
        auto& line = array[aidx];
        os.str("");
        os << "    @ ";
        os << obj.symbol_names[r.sym_index] << " ";
        os << obj.section_names[obj.symbols[r.sym_index].st_shndx] << "@";
        os << "x" << std::hex << obj.symbols[r.sym_index].st_value;
        line.append(os.str());
        line.insert(line.begin(), '>'); // reloc magic to help labeling
    }
}


static void
relocate(
    std::vector<std::string>& array,
    const struct object& obj,
    size_t sidx,
    size_t fidx)
{
    const auto& section = obj.sections[sidx];
    for (size_t i = 0, pc = fidx; i < array.size(); i++, pc += 4) {
        auto reloc = std::find_if(
            section.relocations.begin(),
            section.relocations.end(),
            [=](const section_reloc& token)
            { return token.offset == pc; });
        if (reloc != section.relocations.end()) {
            std::ostringstream os;
            if (array[i].find("BL ") == 0)
                os << std::setw(13) << std::left << "BL" << std::right;
            else if (array[i].find("B ") == 0)
                os << std::setw(13) << std::left << "B" << std::right;
            else os << array[i] << " @ ";
            if (reloc->sym_index < obj.symbol_names.size())
                os << obj.symbol_names[reloc->sym_index];
            else os << "<unknown reloc sym>";
            array[i] = os.str();
        }
    }
}


static void
labelify(
    std::vector<std::string>& array,
    size_t fidx,
    std::string name)
{
    typedef std::pair<size_t,std::string> label_t;
    std::vector<size_t> indices;
    std::vector<label_t> labels;

    // find all branch instructions (except BX and BL)
    for (size_t i = 0; i < array.size(); i++) {
        if (array[i][0] != 'B') continue;
        char c1 = array[i][1];
        char c2 = array[i][2];
        if (c1 == ' ') indices.push_back(i);
        else if (
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
            indices.push_back(i);
    }

    // extract branch addresses (ignoring non-address branches)
    for (auto i : indices) {
        std::istringstream iss(array[i]);
        std::vector<std::string> tokens(
            std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>());
        assert(tokens.size() >= 2);
        char* p;
        unsigned long n = strtoul(tokens[1].c_str(), &p, 16); 
        if (*p == 0) {
            if ((n % 4) != 0) {
                std::cerr << (n % 4) << " != 0" << std::endl;
                assert(false);
            }
            else n /= 4;
            if (n >= array.size()) {
                // hack for times when branch addresses are
                // absolute instead of relative
                n -= (fidx / 4);
            }
            if (n >= array.size()) {
                std::cerr << n << " < " << array.size();
                std::cerr << " " << array[i];
                std::cerr << " function_id: " << (fidx) << ":" << (fidx>>2);
                std::cerr << std::endl;
                assert(false);
            }

            auto f = std::find_if(labels.begin(), labels.end(),
            [=](const label_t& l) { return l.first == n; });

            label_t label_info;
            if (f != labels.end()) label_info = *f;
            else {
                std::ostringstream os;
                os << ".L" << name << "." << labels.size();
                label_info.first = n;
                label_info.second = os.str();
                labels.push_back(label_info);
            }

            // rewrite instruction with new label
            std::ostringstream os;
            os << std::setw(13) << std::left << tokens[0] << std::right;
            os << label_info.second;
            {
                auto size = os.str().size();
                size = 42 - size;
                os << std::setw(size) << "@";
            }
            os << " <" << name << "+0x" << std::hex << (label_info.first) << ">";
            for (size_t k = 3; k < tokens.size(); k++)
                // k=0 (command) k=1 (old address) k=2 (@ for comments)
                os << " " << tokens[i];
            array[i] = os.str();
        }
    }

    // sort labels to make inserting them easier
    std::sort(labels.begin(), labels.end(), std::greater<label_t>());

    for (auto& label : labels)
        assert(label.first < array.size());

    for (const auto& label : labels) {
        if (label.first >= array.size()) {
            std::cerr << "Assert: (" << label.first << " < " << array.size() << ")" << std::endl;
            assert(false);
        }
        std::string label_name = label.second;
        label_name.append(":");
        array.insert(array.begin() + label.first, label_name);
        array.insert(array.begin() + label.first, std::string());
    }    
}


static bool
is_ascii(const std::string& str)
{
    for (auto c : str)
        if ((c < 0x20 || c > 0x7E) &&
            c != '\0' && c != '\n' && c != '\r' && c != '\t')
            return false;
    return true;
}


static std::string
ascii_escape(const std::string& str)
{
    std::ostringstream os;
    for (char c : str) {
             if (c == '\n') os << "\\n";
        else if (c == '\r') os << "\\r";
        else if (c == '\t') os << "\\t";
        else os << c;
    }
    return os.str();
}


static std::string
escape_string(const std::string& str)
{
    std::ostringstream os;
    os << std::setfill('0');
    for (size_t m = 0; m < str.size(); m++) {
        if (str[m] == '\n') os << "\\n";
        else if (str[m] == '\r') os << "\\r";
        else if (str[m] == '\0') os << "\\0";
        else if (str[m] >= ' ' && str[m] <= '~') os << str[m];
        else os << std::hex << std::setw(2) << "\\x" << (0xFF & (int)str[m]);
    }
    return os.str();
}


static void
print_disassembly(struct object& obj)
{
    for (size_t i = 0; i < obj.sections.size(); i++) {
        const auto& section = obj.sections[i];
        if (section.header.sh_type != SHT_PROGBITS) continue;
        if ((section.header.sh_flags & SHF_EXECINSTR) != SHF_EXECINSTR) continue;
        std::vector<funcsym> functions = filter_function_symbols(obj, i);

        if (functions.size() == 0) continue;
        std::cout << "@ " << section.name << "\n\n";

        for (size_t k = 0; k < functions.size(); k++) {
            const auto& f = functions[k];
            std::string chunk = section.raw_data.substr(f.start, f.size);
            auto assembly = disassemble2array(chunk);
            reformat_strings(assembly);
            relocate(assembly, obj, i, f.start);
            labelify(assembly, f.start, f.name);
            std::cout << "FUNC_BEGIN " << f.name << "\n";
            for (size_t j = 0, pc = f.start; j < assembly.size(); j++, pc += 4)
                std::cout << "    " << assembly[j] << "\n";
            std::cout << "FUNC_END " << f.name << "\n\n";
        }

        std::cout << "\n\n";
    }

    for (size_t i = 0; i < obj.sections.size(); i++) {
        const auto& section = obj.sections[i];
        if ((section.header.sh_flags & SHF_EXECINSTR) == SHF_EXECINSTR) continue;

        std::cout << "@ SECTION " << section.name << "\n";

        for (size_t k = 0; k < section.symbol_indices.size(); k++) {
            const auto& symbol = obj.symbols[section.symbol_indices[k]];
            const auto& symname = obj.symbol_names[section.symbol_indices[k]];
            if (symname[0] == '$') continue;
            if (symname.size() == 0) continue;

            if (ELF32_ST_TYPE(symbol.st_info) == STT_OBJECT) {
                std::cout << "@ var " << symname << " at 0x";
                std::cout << std::hex << symbol.st_value << " has size 0x";
                std::cout << std::hex << symbol.st_size << " with value ";
            }
            else if (ELF32_ST_TYPE(symbol.st_info) == STT_NOTYPE) {
                std::cout << "@ sym " << symname << " at 0x";
                std::cout << std::hex << symbol.st_value << " has value ";
            }

            if (section.header.sh_type == SHT_PROGBITS) {
                assert(symbol.st_value < section.raw_data.size());
                if (symbol.st_size == 0) {
                    std::string d(&section.raw_data[symbol.st_value]);
                    d = escape_string(d);
                    std::cout << "\"" << d << "\"";
                }
                else {
                    std::string d = section.raw_data.substr(
                        symbol.st_value, symbol.st_size);
                    if (is_ascii(d)) {
                        d = escape_string(d);
                        std::cout << "\"" << d << "\"";
                    }
                    else {
                        std::cout << std::setfill('0') << "[ ";
                        for (auto c : d) {
                            std::cout << std::setw(2) << std::hex;
                            std::cout << (0xFF & (int)c) << " ";
                        }
                        std::cout << std::setfill(' ') << "]";
                    }
                }
            }
            else std::cout << "<undefined>";
            std::cout << "\n";
        }

        std::cout << "\n\n";
    }
}


static void
print_section(const struct object& obj, std::string name)
{
    auto section = std::find_if(obj.sections.begin(), obj.sections.end(),
        [=](const section_t& sec) { return sec.name == name; });
    if (section == obj.sections.end()) return;

    ELF_print_section_header(obj, section->header_index);
    ELF_print_symbols(obj, section->symbol_indices);
    ELF_print_relocations(obj, section->relocations);
}


static void
print_strings(struct object& obj)
{
    for (size_t i = 0; i < obj.sections.size(); i++) {
        if (obj.sections[i].name == ".rodata" ||
            obj.sections[i].name == ".rodata.str1.4")
            ELF_sort_section_syms_by_value(obj, i);
    }
    for (size_t i = 0; i < obj.sections.size(); i++) {
        const auto& section = obj.sections[i];
        if (section.name == ".rodata") {
            std::cout << "Section: .rodata\n\n";
            for (auto sym_idx : section.symbol_indices) {
                const auto& sym = obj.symbols[sym_idx];
                if (ELF32_ST_TYPE(sym.st_info) != STT_OBJECT) continue;
                if (sym.st_size == 0) continue;

                std::string str(&section.raw_data[sym.st_value], sym.st_size);
                std::cout << std::left;
                std::cout << std::setw(10) << std::hex << obj.symbols[sym_idx].st_value;
                std::cout << std::setw(35) << obj.symbol_names[sym_idx];
                std::cout << std::right << std::dec;
                if (is_ascii(str)) std::cout << ascii_escape(str);
                else {
                    std::cout << std::setfill('0');
                    for (size_t i = 0; i < str.size(); i++)
                        std::cout << std::setw(2) << std::hex << (0xFF & (int)str[i]) << " ";
                    std::cout << std::setfill(' ');
                }
                std::cout << "\n";
            }
            std::cout << "\n\n";
        }
        else if (section.name == ".rodata.str1.4") {
            std::cout << "Section: .rodata.str1.4\n\n";
            for (auto sym_idx : section.symbol_indices) {
                const auto& sym = obj.symbols[sym_idx];
                if (ELF32_ST_TYPE(sym.st_info) != STT_NOTYPE &&
                    ELF32_ST_TYPE(sym.st_info) != STT_OBJECT) continue;

                std::string str(&section.raw_data[sym.st_value]);
                std::cout << std::left;
                std::cout << std::setw(10) << std::hex << obj.symbols[sym_idx].st_value;
                std::cout << std::setw(35) << obj.symbol_names[sym_idx];
                std::cout << std::right << std::dec;
                if (is_ascii(str)) std::cout << ascii_escape(str);
                else {
                    std::cout << std::setfill('0');
                    for (size_t i = 0; i < str.size(); i++)
                        std::cout << std::setw(2) << std::hex << (0xFF & (int)str[i]) << " ";
                    std::cout << std::setfill(' ');
                }
                std::cout << "\n";
            }
            std::cout << "\n\n";
        }
    }
}


static void
main_disassemble(struct object& obj)
{
    for (size_t i = 0; i < obj.sections.size(); i++) {
        const auto& section = obj.sections[i];

        if (section.header.sh_type != SHT_PROGBITS) continue;
        if (!(section.header.sh_flags & SHF_EXECINSTR)) continue;

        std::vector<funcsym> functions = filter_function_symbols(obj, i);
        if (functions.size() == 0) continue;
        std::sort(
            functions.begin(),
            functions.end(),
            [](const funcsym& a, const funcsym& b) {
                return a.start < b.start;
            }
        );

        std::cout << "@ " << section.name << "\n\n";

        auto asm_strings = disassemble2array(section.raw_data);
        reformat_strings(asm_strings);
        relocate_syms(asm_strings, obj, i);

        for (size_t k = 0; k < functions.size(); k++) {
            const auto& f = functions[k];
            auto f_start = f.start / 4;
            auto f_end = ((f.start + f.size) / 4);

            if (!((f_start < asm_strings.size()) && (f_end <= asm_strings.size()))) {
                std::cerr << "ERROR: \n";
                std::cerr << "f_start: " << f_start << " < " << asm_strings.size() << "\n";
                std::cerr << "f_end:   " << f_end   << " < " << asm_strings.size() << std::endl;
            }
            assert((f_start < asm_strings.size()) && (f_end <= asm_strings.size()));

            std::vector<std::string> func_asm(
                asm_strings.begin() + f_start,
                asm_strings.begin() + f_end
            );
            labelify(func_asm, f.start, f.name);

            std::cout << "FUNC_BEGIN " << f.name << "\n";
            for (const auto& line : func_asm)
                std::cout << "    " << line << "\n";
            std::cout << "FUNC_END " << f.name << "\n\n";    
        }

        std::cout << "\n\n";
    }
}


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <obj-file> [p/s/d/t]\n";
        std::cout << "t: print target section; 't .bss' - default .text\n";
        return -1;
    }

    std::ifstream objfile(argv[1]);
    if (!objfile.is_open()) {
        std::cout << "could not open file " << argv[1] << std::endl;
        return -1;
    }
    struct object obj = ELF_parse(objfile);
    objfile.close();

    if (argc > 2) {
        switch (argv[2][0]) {
        case 'p': ELF_print(obj); break;
        case 's': ELF_print_sections(obj); break;
        case 'd': print_strings(obj); break;
        case 't': {
            std::string target = ".text";
            if (argc > 3) target = std::string(argv[3]);
            for (size_t i = 0; i < obj.sections.size(); i++) {
                if (obj.sections[i].name == target)
                    ELF_sort_section_syms_by_value(obj, i);
            }
            print_section(obj, target);
        } break;
        default: main_disassemble(obj); break;
        }
    }
    else {
        decompile(obj);
    }

    return 0;
}
