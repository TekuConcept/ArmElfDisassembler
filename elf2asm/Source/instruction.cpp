/**
 * Created by TekuConcept on March 24, 2020
 */

#include "instruction.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iterator>

using namespace arm;


command_t
instruction_t::command_from_string(
    const std::string& string,
    size_t offset)
{
    if (string.size() == 0) return command_t::UNKNOWN;

    std::string s(string.begin() + offset, string.end());
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return std::toupper(c); });

    if (s.size() >= 5) {
        if (s[0] == 'U' && s[1] == 'M') {
            if (s[2] == 'A' && s[3] == 'A' && s[4] == 'L')
                return command_t::UMAAL;
            else if (s[2] == 'L' && s[3] == 'A' && s[4] == 'L')
                return command_t::UMLAL;
            else if (s[2] == 'U' && s[3] == 'L' && s[4] == 'L')
                return command_t::UMULL;
        }
    }
    if (s.size() >= 4) {
        if (s[0] == 'B') {
            if (s[1] == 'K' && s[2] == 'P' && s[3] == 'T')
                return command_t::BKPT;
        }
        else if (s[0] == 'C') {
            if (s[1] == 'B' && s[2] == 'N' && s[3] == 'Z')
                return command_t::CBNZ;
        }
        else if (s[0] == 'M') {
            if (s[1] == 'O' && s[2] == 'V' && s[3] == 'T')
                return command_t::MOVT;
        }
        else if (s[0] == 'P') {
            if (s[1] == 'U' && s[2] == 'S' && s[3] == 'H')
                return command_t::PUSH;
        }
        else if (s[0] == 'S') {
            if (s[1] == 'B' && s[2] == 'F' && s[3] == 'X')
                return command_t::SBFX;
            else if (s[1] == 'D' && s[2] == 'I' && s[3] == 'V')
                return command_t::SDIV;
        }
        else if (s[0] == 'U') {
            if (s[1] == 'D') {
                if (s[2] == 'I' && s[3] == 'V') return command_t::UDIV;
            }
            else if (s[1] == 'X' && s[2] == 'T') {
                if (s[3] == 'B') return command_t::UXTB;
                else if (s[3] == 'H') return command_t::UXTH;
            }
            else if (s[1] == 'B' && s[2] == 'F' && s[3] == 'X')
                return command_t::UBFX;
        }
    }
    if (s.size() >= 3) {
        if (s[0] == 'A') {
            if (s[1] == 'D') {
                if (s[2] == 'C') return command_t::ADC;
                else if (s[2] == 'D') return command_t::ADD;
                // else if (s[2] == 'R') return command_t::ADR;
            }
            else if (s[1] == 'N') {
                if (s[2] == 'D') return command_t::AND;
            }
            else if (s[1] == 'S') {
                if (s[2] == 'R') return command_t::ASR;
            }
        }
        else if (s[0] == 'B') {
            if (s[1] == 'F') {
                if (s[2] == 'C') return command_t::BFC;
                else if (s[2] == 'I') return command_t::BFI;
            }
            else if (s[1] == 'I') {
                if (s[2] == 'C') return command_t::BIC;
            }
            else if (s[1] == 'L') {
                if (s[2] == 'X') return command_t::BLX;
                if (s[2] == 'E' || s[2] == 'T' || s[2] == 'S' || s[2] == 'O')
                    return command_t::B;
            }
        }
        else if (s[0] == 'C') {
            if (s[1] == 'B') {
                if (s[2] == 'Z') return command_t::CBZ;
            }
            else if (s[1] == 'L') {
                if (s[2] == 'Z') return command_t::CLZ;
            }
            else if (s[1] == 'M') {
                if (s[2] == 'P') return command_t::CMP;
                else if (s[2] == 'N') return command_t::CMN;
            }
        }
        else if (s[0] == 'E') {
            if (s[1] == 'O' && s[2] == 'R') return command_t::EOR;
        }
        else if (s[0] == 'L') {
            if (s[1] == 'D') {
                if (s[2] == 'M') return command_t::LDM;
                else if (s[2] == 'R') return command_t::LDR;
            }
            else if (s[1] == 'S') {
                if (s[2] == 'L') return command_t::LSL;
                else if (s[2] == 'R') return command_t::LSR;
            }
        }
        else if (s[0] == 'M') {
            if (s[1] == 'L') {
                if (s[2] == 'A') return command_t::MLA;
                else if (s[2] == 'S') return command_t::MLS;
            }
            else if (s[1] == 'O') {
                if (s[2] == 'V') return command_t::MOV;
            }
            else if (s[1] == 'U') {
                if (s[2] == 'L') return command_t::MUL;
            }
            else if (s[1] == 'V') {
                if (s[2] == 'N') return command_t::MVN;
            }
        }
        else if (s[0] == 'N') {
            if (s[1] == 'O' && s[2] == 'P')
                return command_t::NOP;
        }
        else if (s[0] == 'O') {
            if (s[1] == 'R' && s[2] == 'R')
                return command_t::ORR;
        }
        else if (s[0] == 'P') {
            if (s[1] == 'O' && s[2] == 'P')
                return command_t::POP;
        }
        else if (s[0] == 'R') {
            if (s[1] == 'O') {
                if (s[2] == 'R') return command_t::ROR;
            }
            else if (s[1] == 'S') {
                if (s[2] == 'B') return command_t::RSB;
                else if (s[2] == 'C') return command_t::RSC;
            }
        }
        else if (s[0] == 'S') {
            if (s[1] == 'B') {
                if (s[2] == 'C') return command_t::SBC;
            }
            else if (s[1] == 'T') {
                if (s[2] == 'M') return command_t::STM;
                else if (s[2] == 'R') return command_t::STR;
            }
            else if (s[1] == 'U') {
                if (s[2] == 'B') return command_t::SUB;
            }
        }
        else if (s[0] == 'T') {
            if (s[1] == 'E' && s[2] == 'Q')
                return command_t::TEQ;
            else if (s[1] == 'S' && s[2] == 'T')
                return command_t::TST;
        }
    }
    if (s.size() >= 2) {
        if (s[0] == 'B') {
            if (s[1] == 'L') return command_t::BL;
            else if (s[1] == 'X') return command_t::BX;
        }
    }
    if (s.size() >= 1) {
        if (s[0] == 'B') return command_t::B;
    }

    return command_t::UNKNOWN;
}


std::string
instruction_t::command_to_string(command_t c)
{
    switch (c) {
    case command_t::B:     return "B";
    case command_t::BL:    return "BL";
    case command_t::BX:    return "BX";
    case command_t::ADC:   return "ADC";
    case command_t::ADD:   return "ADD";
    // case command_t::ADR:   return "ADR";
    case command_t::AND:   return "AND";
    case command_t::ASR:   return "ASR";
    case command_t::BFC:   return "BFC";
    case command_t::BFI:   return "BFI";
    case command_t::BIC:   return "BIC";
    case command_t::BLX:   return "BLX";
    case command_t::CBZ:   return "CBZ";
    case command_t::CLZ:   return "CLZ";
    case command_t::CMP:   return "CMP";
    case command_t::CMN:   return "CMN";
    case command_t::EOR:   return "EOR";
    case command_t::LDM:   return "LDM";
    case command_t::LDR:   return "LDR";
    case command_t::LSL:   return "LSL";
    case command_t::LSR:   return "LSR";
    case command_t::MLA:   return "MLA";
    case command_t::MLS:   return "MLS";
    case command_t::MOV:   return "MOV";
    case command_t::MUL:   return "MUL";
    case command_t::MVN:   return "MVN";
    case command_t::NOP:   return "NOP";
    case command_t::ORR:   return "ORR";
    case command_t::POP:   return "POP";
    case command_t::ROR:   return "ROR";
    case command_t::RSB:   return "RSB";
    case command_t::RSC:   return "RSC";
    case command_t::SBC:   return "SBC";
    case command_t::STM:   return "STM";
    case command_t::STR:   return "STR";
    case command_t::SUB:   return "SUB";
    case command_t::TEQ:   return "TEQ";
    case command_t::TST:   return "TST";
    case command_t::BKPT:  return "BKPT";
    case command_t::CBNZ:  return "CBNZ";
    case command_t::MOVT:  return "MOVT";
    case command_t::PUSH:  return "PUSH";
    case command_t::SBFX:  return "SBFX";
    case command_t::SDIV:  return "SDIV";
    case command_t::UBFX:  return "UBFX";
    case command_t::UDIV:  return "UDIV";
    case command_t::UXTB:  return "UXTB";
    case command_t::UXTH:  return "UXTH";
    case command_t::UMAAL: return "UMAAL";
    case command_t::UMLAL: return "UMLAL";
    case command_t::UMULL: return "UMULL";
    default: return "<unknown>";
    }
}


size_t
instruction_t::command_string_size(command_t c)
{
    switch (c) {
    case command_t::B: return 1;
    case command_t::BL: case command_t::BX: return 2;
    case command_t::ADC: case command_t::ADD: //case command_t::ADR:
    case command_t::AND: case command_t::ASR: case command_t::BFC:
    case command_t::BFI: case command_t::BIC: case command_t::BLX:
    case command_t::CBZ: case command_t::CLZ: case command_t::CMP:
    case command_t::CMN: case command_t::EOR: case command_t::LDM:
    case command_t::LDR: case command_t::LSL: case command_t::LSR:
    case command_t::MLA: case command_t::MLS: case command_t::MOV:
    case command_t::MUL: case command_t::MVN: case command_t::NOP:
    case command_t::ORR: case command_t::POP: case command_t::ROR:
    case command_t::RSB: case command_t::RSC: case command_t::SBC:
    case command_t::STM: case command_t::STR: case command_t::SUB:
    case command_t::TEQ: case command_t::TST: return 3;
    case command_t::BKPT: case command_t::CBNZ: case command_t::MOVT:
    case command_t::PUSH: case command_t::SBFX: case command_t::SDIV:
    case command_t::UBFX: case command_t::UDIV: case command_t::UXTB:
    case command_t::UXTH: return 4;
    case command_t::UMAAL: case command_t::UMLAL: case command_t::UMULL: return 5;
    default: return 0;
    }
}


condition_t
instruction_t::condition_from_chars(char a, char b)
{
    a = std::toupper(a);
    b = std::toupper(b);

    if (a == 'A') {
        if (b == 'L') return condition_t::AL;
    }
    else if (a == 'C') {
        if (b == 'C') return condition_t::CC;
        else if (b == 'S') return condition_t::CS;
    }
    else if (a == 'E') {
        if (b == 'Q') return condition_t::EQ;
    }
    else if (a == 'G') {
        if (b == 'E') return condition_t::GE;
        else if (b == 'T') return condition_t::GT;
    }
    else if (a == 'H') {
        if (b == 'I') return condition_t::HI;
        else if (b == 'S') return condition_t::HS;
    }
    else if (a == 'L') {
        if (b == 'E') return condition_t::LE;
        else if (b == 'O') return condition_t::LO;
        else if (b == 'S') return condition_t::LS;
        else if (b == 'T') return condition_t::LT;
    }
    else if (a == 'M') {
        if (b == 'I') return condition_t::MI;
    }
    else if (a == 'N') {
        if (b == 'E') return condition_t::NE;
    }
    else if (a == 'P') {
        if (b == 'L') return condition_t::PL;
    }
    else if (a == 'V') {
        if (b == 'S') return condition_t::VS;
        else if (b == 'C') return condition_t::VC;
    }
    return condition_t::UNSPECIFIED;
}


std::string
instruction_t::condition_to_string(condition_t c)
{
    switch (c) {
    case condition_t::EQ: return "EQ";
    case condition_t::NE: return "NE";
    case condition_t::CS: return "CS";
    case condition_t::CC: return "CC";
    case condition_t::VS: return "VS";
    case condition_t::VC: return "VC";
    case condition_t::HS: return "HS";
    case condition_t::HI: return "HI";
    case condition_t::LS: return "LS";
    case condition_t::LO: return "LO";
    case condition_t::LT: return "LT";
    case condition_t::LE: return "LE";
    case condition_t::GT: return "GT";
    case condition_t::GE: return "GE";
    case condition_t::MI: return "MI";
    case condition_t::PL: return "PL";
    }
    return "";
}


std::string
instruction_t::string_from_condition(condition_t c)
{
    switch (c) {
    case condition_t::AL: return "";
    case condition_t::EQ: return "EQ";
    case condition_t::NE: return "NE";
    case condition_t::CS: return "CS";
    case condition_t::HS: return "HS";
    case condition_t::CC: return "CC";
    case condition_t::LO: return "LO";
    case condition_t::MI: return "MI";
    case condition_t::PL: return "PL";
    case condition_t::VS: return "VS";
    case condition_t::VC: return "VC";
    case condition_t::HI: return "HI";
    case condition_t::LS: return "LS";
    case condition_t::GE: return "GE";
    case condition_t::LT: return "LT";
    case condition_t::GT: return "GT";
    case condition_t::LE: return "LE";
    default: return "";
    }
}


instruction_t
instruction_t::parse(const std::string& line)
{
    instruction_t instruction;
    std::string source = line;
    size_t idx;

    std::replace(source.begin(), source.end(), '\t', ' ');
    std::replace(source.begin(), source.end(), ',', ' ');
    std::istringstream iss(source);
    std::vector<std::string> tokens(
        std::istream_iterator<std::string>{iss},
        std::istream_iterator<std::string>());
    if (tokens.size() == 0) throw std::runtime_error("not a valid instruction");

    idx = tokens.size(); // when there is no comment
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].front() == '@') {
            idx = i;
            break;
        }
    }
    instruction.m_comments = "";
    for (size_t i = idx; i < tokens.size(); i++) {
        if (instruction.m_comments.size() > 0)
            instruction.m_comments += std::string(" ");
        instruction.m_comments += tokens[i];
    }
    while (tokens.size() > idx) tokens.pop_back();

    instruction.m_original  = source;
    instruction.m_command   = command_from_string(tokens[0]);
    instruction.m_set       = false;
    instruction.m_condition = condition_t::UNSPECIFIED;
    instruction.m_info.m_increment_order    = address_increment_order::IA;
    instruction.m_info.m_register_data_size = register_data_size::W;
    instruction.m_order     = 0;
    idx = command_string_size(instruction.m_command);

    if (instruction.m_command == command_t::UNKNOWN)
        throw std::runtime_error(std::string("unsupported command ") + tokens[0]);

    if (idx < tokens[0].size()) {
        auto c1 = tokens[0][idx], c2 = tokens[0][idx + 1];
        switch (instruction.m_command) {
        case command_t::MOV: if (c1 == 'W') idx++; break;
        case command_t::LDM:
        case command_t::STM:
            if (c1 == 'I') {
                if (c2 == 'A')      instruction.m_info.m_increment_order = address_increment_order::IA;
                else if (c2 == 'B') instruction.m_info.m_increment_order = address_increment_order::IB;
            }
            else if (c1 == 'D') {
                if (c2 == 'A')      instruction.m_info.m_increment_order = address_increment_order::DA;
                else if (c2 == 'B') instruction.m_info.m_increment_order = address_increment_order::DB;
            }
            idx += 2;
            break;
        case command_t::LDR:
        case command_t::STR:
            if (c1 == 'S') {
                if (c2 == 'B') {
                    instruction.m_info.m_register_data_size = register_data_size::SB;
                    idx += 2;
                }
                else if (c2 == 'H') {
                    instruction.m_info.m_register_data_size = register_data_size::SH;
                    idx += 2;
                }
            }
            else if (c1 == 'B') {
                instruction.m_info.m_register_data_size = register_data_size::B;
                idx++;
            }
            else if (c1 == 'H') {
                if (c2 != 'I' && c2 != 'S') {
                    instruction.m_info.m_register_data_size = register_data_size::H;
                    idx++;
                }
            }
            else if (c1 == 'D') {
                instruction.m_info.m_register_data_size = register_data_size::D;
                idx++;
            }
            break;
        default:
            if (c1 == 'S') {
                instruction.m_set = true;
                idx++;
            }
            break;
        }
    }

    if (idx < (tokens[0].size() - 1)) {
        instruction.m_condition = condition_from_chars(
            tokens[0][idx + 0],
            tokens[0][idx + 1]
        );
        idx += 2;
    }

    if (idx != tokens[0].size())
        throw std::runtime_error(std::string("undefined instruction detected ") + tokens[0]);

    switch (instruction.m_command) {
    case command_t::LDR:
    case command_t::STR:
        if (tokens.size() < 3) throw std::runtime_error(std::string("invalid number of arguments ") + tokens[0]);
        instruction.m_params.push_back(tokens[1]);
        if (tokens.back().back() == '!') {
            instruction.m_order = 1;
            tokens.back().erase(std::remove(tokens.back().begin(), tokens.back().end(), '!'), tokens.back().end());
        }
        else if (tokens[2].front() == '[' && tokens[2].back() == ']' && tokens.size() > 3)
            instruction.m_order = 2;
        std::for_each(tokens.begin() + 2, tokens.end(), [&](std::string token) {
            token.erase(std::remove(token.begin(), token.end(), '['), token.end());
            token.erase(std::remove(token.begin(), token.end(), ']'), token.end());
            instruction.m_params.push_back(token);
        });
        break;
    case command_t::LDM:
    case command_t::STM: {
        if (tokens.size() < 3) throw std::runtime_error(std::string("invalid number of arguments ") + tokens[0]);
        auto tokenptr = tokens.begin() + 1;
        if (tokens[1].back() == '!') {
            tokenptr++;
            instruction.m_order = 1;
            tokens[1].erase(std::remove(tokens[1].begin(), tokens[1].end(), '!'), tokens[1].end());
            instruction.m_params.push_back(tokens[1]);
        }
        std::for_each(tokenptr, tokens.end(), [&](std::string token) {
            token.erase(std::remove(token.begin(), token.end(), '{'), token.end());
            token.erase(std::remove(token.begin(), token.end(), '}'), token.end());
            instruction.m_params.push_back(token);
        });
    } break;
    default:
        std::for_each(tokens.begin() + 1, tokens.end(), [&](std::string token) {
            token.erase(std::remove(token.begin(), token.end(), '{'), token.end());
            token.erase(std::remove(token.begin(), token.end(), '}'), token.end());
            instruction.m_params.push_back(token);
        });
        break;
    }

    return instruction;
}


std::string
instruction_t::tag_string() const
{
    std::istringstream iss(this->m_original);
    std::vector<std::string> tokens(
        std::istream_iterator<std::string>{iss},
        std::istream_iterator<std::string>());
    return tokens.front();
}


std::string
instruction_t::to_c() const
{
    std::ostringstream os;
    auto fstr_cond = [](condition_t cond) {
        switch (cond) {
        case condition_t::CC: return "if (a <  b) ";
        case condition_t::CS: return "if (a >= b) ";
        case condition_t::EQ: return "if (a == b) ";
        case condition_t::NE: return "if (a != b) ";
        case condition_t::MI: return "if (a <  0) ";
        case condition_t::PL: return "if (a >= 0) ";
        case condition_t::GE: return "if ((signed)a >= (signed)b) ";
        case condition_t::GT: return "if ((signed)a >  (signed)b) ";
        case condition_t::LE: return "if ((signed)a <= (signed)b) ";
        case condition_t::LT: return "if ((signed)a <  (signed)b) ";
        case condition_t::HS: return "if ((unsigned)a >= (unsigned)b) ";
        case condition_t::HI: return "if ((unsigned)a >  (unsigned)b) ";
        case condition_t::LS: return "if ((unsigned)a <= (unsigned)b) ";
        case condition_t::LO: return "if ((unsigned)a <  (unsigned)b) ";
        case condition_t::VC: return "if (!overflow) ";
        case condition_t::VS: return "if (overflow) ";
        default: return ""; // AL, UNSPECIFIED
        }
    };

    os << fstr_cond(this->m_condition);
    switch (this->m_command) {
    case command_t::ADC:
        // ADC{S}{cond} {Rd}, Rn, Operand2
        if (this->m_params.size() == 2)
            os << m_params[0] << " += " << m_params[1] << " + CARRY";
        else if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " + " << m_params[2] << " + CARRY";
        else goto param_error;
        break;
    case command_t::ADD:
        // ADD{S}{cond} {Rd}, Rn, Operand2
        if (this->m_params.size() == 2)
            os << m_params[0] << " += " << m_params[1];
        else if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " + " << m_params[2];
        else if (this->m_params.size() == 5) {
            if (m_params[3] == "LSL") os << m_params[0] << " = " << m_params[1] << " + (" << m_params[2] << " << " << m_params[4] << ")";
            else if (m_params[3] == "LSR" || m_params[3] == "ASR") os << m_params[0] << " = " << m_params[1] << " + (" << m_params[2] << " >> " << m_params[4] << ")";
            else if (m_params[3] == "ROR") os << m_params[0] << " = " << m_params[1] << " + std::rotr(" << m_params[2] << ", " << m_params[4] << ")";
            else goto param_error;
        }
        else goto param_error;
        break;
    case command_t::AND:
        // AND{S}{cond} Rd, Rn, Operand2
        if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " & " << m_params[2];
        else if (this->m_params.size() == 4) {
            if (m_params[3] == "RRX") os << m_params[0] << " = " << m_params[1] << " & (" << m_params[2] << " >> 1)";
            else goto param_error;
        }
        else if (this->m_params.size() == 5) {
            if (m_params[3] == "LSL") os << m_params[0] << " = " << m_params[1] << " & (" << m_params[2] << " << " << m_params[4] << ")";
            else if (m_params[3] == "LSR" || m_params[3] == "ASR") os << m_params[0] << " = " << m_params[1] << " & (" << m_params[2] << " >> " << m_params[4] << ")";
            else if (m_params[3] == "ROR") os << m_params[0] << " = " << m_params[1] << " & std::rotr(" << m_params[2] << ", " << m_params[4] << ")";
            else goto param_error;
        }
        else goto param_error;
        break;
    case command_t::ASR:
        // ASR{S}{cond} Rd, Rm, Rs
        if (this->m_params.size() == 3)
            os << m_params[0] << " = (signed)" << m_params[1] << " >> " << m_params[2];
        else goto param_error;
        break;
    case command_t::B:
        // B{cond} label
        if (this->m_params.size() == 1)
            os << "goto " << m_params[0];
        else goto param_error;
        break;
    case command_t::BFC:
        // BFC{cond} Rd, #lsb, #width
        // Rd &= ~((2^#width - 1) << #lsb)
        if (this->m_params.size() == 3)
            os << m_params[0] << " &= ~(bitfield(" << m_params[2] << ") << " << m_params[1] << ")";
        else goto param_error;
        break;
    case command_t::BFI:
        // BFI{cond} Rd, Rn, #lsb, #width
        // Rd = (Rd & (~((2^#width - 1) << #lsb))) | (Rn & ((2^#width - 1) << #lsb))
        if (this->m_params.size() == 4) {
            os << m_params[0] << " = (";
            os << m_params[0] << " & ~(bitfield(" << m_params[3] << ") << " << m_params[2] << ")) | (";
            os << m_params[1] << " &  (bitfield(" << m_params[3] << ") << " << m_params[2] << "))";
        }
        else goto param_error;
        break;
    case command_t::BIC:
        // BIC{S}{cond} Rd, Rn, Operand2
        if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " & ~(" << m_params[2] << ")";
        else goto param_error;
        break;
    case command_t::BL:
    case command_t::BLX:
        // BL{cond} label
        // BLX{cond} label
        // BLX{cond} Rm
        if (this->m_params.size() == 1)
            os << "(fn)(" << m_params[0] << ")()";
        else goto param_error;
        break;
    case command_t::BX:
        // BX{cond} Rm
        if (this->m_params.size() == 1)
            os << "return [" << m_params[0] << "]";
        else goto param_error;
        break;
    case command_t::CBZ:
        // CBZ Rn, label
        if (this->m_params.size() == 2)
            os << "if (" << m_params[0] << " == 0) goto " << m_params[1];
        else goto param_error;
        break;
    case command_t::CBNZ:
        // CBNZ Rn, label
        if (this->m_params.size() == 2)
            os << m_params[0] << " &= ~(bitfield(" << m_params[2] << ") << " << m_params[1] << ")";
        else goto param_error;
        break;
    case command_t::CLZ:
        // CLZ{cond} Rd, Rm
        if (this->m_params.size() == 2)
            os << m_params[0] << " = (" << m_params[1] << " == 0 ? 32 : __builtin_clz(" << m_params[1] << ")";
        else goto param_error;
        break;
    case command_t::CMP:
        // CMP{cond} Rn, Operand2
        if (this->m_params.size() == 2)
            os << "if ((" << m_params[0] << " - " << m_params[1] << ") ? X) ...";
        else goto param_error;
        break;
    case command_t::CMN:
        // CMN{cond} Rn, Operand2
        if (this->m_params.size() == 2)
            os << "if ((" << m_params[0] << " + " << m_params[1] << ") ? X) ...";
        else goto param_error;
        break;
    case command_t::EOR:
        // EOR{S}{cond} Rd, Rn, Operand2
        if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " ^ " << m_params[2];
        else if (this->m_params.size() == 5) {
            if (m_params[3] == "LSL") os << m_params[0] << " = " << m_params[1] << " ^ (" << m_params[2] << " << " << m_params[4] << ")";
            else if (m_params[3] == "LSR" || m_params[3] == "ASR") os << m_params[0] << " = " << m_params[1] << " ^ (" << m_params[2] << " >> " << m_params[4] << ")";
            else if (m_params[3] == "ROR") os << m_params[0] << " = " << m_params[1] << " ^ std::rotr(" << m_params[2] << ", " << m_params[4] << ")";
            else goto param_error;
        }
        else goto param_error;
        break;
    case command_t::LDM:
        // LDM{addr_mode}{cond} Rn{!}, reglist{^}
        if (this->m_params.size() >= 2) {
            ssize_t k;
            auto o = "";
            switch (m_info.m_increment_order) {
            case address_increment_order::IA: k = 0; o = " + "; break;
            case address_increment_order::DA: k = 0; o = " - "; break;
            case address_increment_order::IB: k = 4; o = " + "; break;
            case address_increment_order::DB: k = 4; o = " - "; break;
            }
            for (size_t n = 1; n < m_params.size(); n++) {
                os << m_params[n] << " = *(" << m_params[0] << o << k << "); ";
                k += 4;
            }
            if (m_order == 1) os << m_params[0] << " += " << (k - 4);
        }
        else goto param_error;
        break;
    case command_t::LDR: {
        // LDR{type}{cond} Rt, [Rn {, #offset}]
        // LDR{type}{cond} Rt, [Rn, #offset]!
        // LDR{type}{cond} Rt, [Rn], #offset
        // LDR{type}{cond} Rt, [Rn, ±Rm {, shift}]
        // LDR{type}{cond} Rt, [Rn, ±Rm {, shift}]!
        // LDR{type}{cond} Rt, [Rn], ±Rm {, shift}
        std::string note;
        switch (m_info.m_register_data_size) {
        case register_data_size::B:  note = "(uint8_t)";  break;
        case register_data_size::H:  note = "(uint16_t)"; break;
        case register_data_size::W:  note = "";           break;
        case register_data_size::D:  note = "(uint64_t)"; break;
        case register_data_size::SB: note = "(int8_t)";   break;
        case register_data_size::SH: note = "(int16_t)";  break;
        }
        if (this->m_params.size() == 2)
            os << m_params[0] << " = *" << note << "(" << m_params[1] << ")";
        else if (this->m_params.size() == 3) {
            if (m_order == 1)
                os << m_params[0] << " = *" << note << "(" << m_params[1] << " + " << m_params[2] << "); " << m_params[1] << " += " << m_params[2];
            else if (m_order == 2)
                os << m_params[0] << " = *" << note << "(" << m_params[1] << "); " << m_params[1] << " += " << m_params[2];
            else os << m_params[0] << " = *" << note << "(" << m_params[1] << " + " << m_params[2] << ")";
        }
        else if (this->m_params.size() == 5) {
            const auto&
                a = m_params[0],
                b = m_params[1],
                c = m_params[2],
                d = m_params[4];
            if (m_params[3] == "ROR") {
                if (m_order == 1)
                    os << a << " = *" << note << "(" << b << " + std::rotr(" << c << ", " << d << ")); " << b << " += std::rotr(" << c << ", " << d << ")";
                else if (m_order == 2)
                    os << a << " = *" << note << "(" << b << "); " << b << " += std::rotr(" << c << ", " << d << ")";
                else os << a << " = *" << note << "(" << b << " + std::rotr(" << c << ", " << d << "))";
            }
            else if (m_params[3] == "ASR" || m_params[3] == "LSR") {
                if (m_order == 1)
                    os << a << " = *" << note << "(" << b << " + (" << c << " >> " << d << ")); " << b << " += (" << c << " >> " << d << ")";
                else if (m_order == 2)
                    os << a << " = *" << note << "(" << b << "); " << b << " += (" << c << " >> " << d << ")";
                else os << a << " = *" << note << "(" << b << " + (" << c << " >> " << d << "))";
            }
            else if (m_params[3] == "LSL") {
                if (m_order == 1)
                    os << a << " = *" << note << "(" << b << " + (" << c << " << " << d << ")); " << b << " += (" << c << " << " << d << ")";
                else if (m_order == 2)
                    os << a << " = *" << note << "(" << b << "); " << b << " += (" << c << " << " << d << ")";
                else os << a << " = *" << note << "(" << b << " + (" << c << " << " << d << "))";
            }
            else goto param_error;
        }
        else goto param_error;
    } break;
    case command_t::LSL:
        // LSL{S}{cond} Rd, Rm, Rs
        if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " << " << m_params[2];
        else goto param_error;
        break;
    case command_t::LSR:
        // LSR{S}{cond} Rd, Rm, Rs
        if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " >> " << m_params[2];
        else goto param_error;
        break;
    case command_t::MLA:
        // MLA{S}{cond} Rd, Rn, Rm, Ra
        if (this->m_params.size() == 4)
            os << m_params[0] << " = (" << m_params[1] << " * " << m_params[2] << ") + " << m_params[3];
        else goto param_error;
        break;
    case command_t::MLS:
        // MLS{cond} Rd, Rn, Rm, Ra
        if (this->m_params.size() == 4)
            os << m_params[0] << " = (" << m_params[1] << " * " << m_params[2] << ") - " << m_params[3];
        else goto param_error;
        break;
    case command_t::MOV:
        // MOV{S}{cond} Rd, Operand2
        if (this->m_params.size() == 2)
            os << m_params[0] << " = " << m_params[1];
        else goto param_error;
        break;
    case command_t::MOVT:
        // MOVT{cond} Rd, #imm16
        if (this->m_params.size() == 2)
            os << m_params[0] << " = (0xFFFF & " << m_params[0] << ") | (" << m_params[1] << " << 16)";
        else goto param_error;
        break;
    case command_t::MUL:
        // MUL{S}{cond} {Rd}, Rn, Rm
        if (this->m_params.size() == 2)
            os << m_params[0] << " *= " << m_params[1];
        else if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " * " << m_params[2];
        else goto param_error;
        break;
    case command_t::MVN:
        // MVN{S}{cond} Rd, Operand2
        if (this->m_params.size() == 2)
            os << m_params[0] << " = ~" << m_params[1];
        else goto param_error;
        break;
    case command_t::ORR:
        // ORR{S}{cond} Rd, Rn, Operand2
        if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " | " << m_params[2];
        else if (this->m_params.size() == 5) {
            if (m_params[3] == "LSL") os << m_params[0] << " = " << m_params[1] << " | (" << m_params[2] << " << " << m_params[4] << ")";
            else if (m_params[3] == "LSR" || m_params[3] == "ASR") os << m_params[0] << " = " << m_params[1] << " | (" << m_params[2] << " >> " << m_params[4] << ")";
            else if (m_params[3] == "ROR") os << m_params[0] << " = " << m_params[1] << " | std::rotr(" << m_params[2] << ", " << m_params[4] << ")";
            else goto param_error;
        }
        else goto param_error;
        break;
    case command_t::ROR:
        // ROR{S}{cond} Rd, Rm, Rs
        if (this->m_params.size() == 3)
            os << m_params[0] << " = std::rotr(" << m_params[1] << ", " << m_params[2] << ")";
        else goto param_error;
        break;
    case command_t::RSB:
        // RSB{S}{cond} {Rd}, Rn, Operand2
        if (this->m_params.size() == 2)
            os << m_params[0] << " = " << m_params[1] << " - " << m_params[0];
        else if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[2] << " - " << m_params[1];
        else goto param_error;
        break;
    case command_t::RSC:
        // RSC{S}{cond} {Rd}, Rn, Operand2
        if (this->m_params.size() == 2)
            os << m_params[0] << " = " << m_params[1] << " - " << m_params[0] << " - !CARRY";
        else if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[2] << " - " << m_params[1] << " - !CARRY";
        else goto param_error;
        break;
    case command_t::SBC:
        // SBC{S}{cond} {Rd}, Rn, Operand2
        if (this->m_params.size() == 2)
            os << m_params[0] << " -= " << m_params[1] << " - !CARRY";
        else if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " - " << m_params[2] << " - !CARRY";
        else goto param_error;
        break;
    case command_t::SBFX:
        // SBFX{cond} Rd, Rn, #lsb, #width
        if (this->m_params.size() == 4)
            os << m_params[0] << " = ((signed)" << m_params[1] << " & ~(bitfield(" << m_params[3] << ") << " << m_params[2] << ")";
        else goto param_error;
        break;
    case command_t::SDIV:
        // SDIV{cond} {Rd}, Rn, Rm
        if (this->m_params.size() == 2)
            os << m_params[0] << " /= (signed)" << m_params[1];
        else if (this->m_params.size() == 3)
            os << m_params[0] << " = (signed)" << m_params[1] << " / (signed)" << m_params[2];
        else goto param_error;
        break;
    case command_t::STM:
        // STM{addr_mode}{cond} Rn{!}, reglist{^}
        if (this->m_params.size() >= 2) {
            ssize_t k;
            auto o = "";
            switch (m_info.m_increment_order) {
            case address_increment_order::IA: k = 0; o = " + "; break;
            case address_increment_order::DA: k = 0; o = " - "; break;
            case address_increment_order::IB: k = 4; o = " + "; break;
            case address_increment_order::DB: k = 4; o = " - "; break;
            }
            for (size_t n = 1; n < m_params.size(); n++) {
                os << "*(" << m_params[0] << o << k << ") = " << m_params[n] << "; ";
                k += 4;
            }
            if (m_order == 1) os << m_params[0] << " += " << (k - 4);
        }
        else goto param_error;
        break;
    case command_t::STR: {
        // STR{type}{cond} Rt, [Rn {, #offset}]
        // STR{type}{cond} Rt, [Rn, #offset]!
        // STR{type}{cond} Rt, [Rn], #offset
        // STR{type}{cond} Rt, [Rn, ±Rm {, shift}]
        // STR{type}{cond} Rt, [Rn, ±Rm {, shift}]!
        // STR{type}{cond} Rt, [Rn], ±Rm {, shift}
        std::string note;
        switch (m_info.m_register_data_size) {
        case register_data_size::B:  note = "(uint8_t)";  break;
        case register_data_size::H:  note = "(uint16_t)"; break;
        case register_data_size::W:  note = "";           break;
        case register_data_size::D:  note = "(uint64_t)"; break;
        case register_data_size::SB: note = "(int8_t)";   break;
        case register_data_size::SH: note = "(int16_t)";  break;
        }
        if (this->m_params.size() == 2)
            os << "*" << note << "(" << m_params[1] << ") = " << m_params[0];
        else if (this->m_params.size() == 3) {
            if (m_order == 1)
                os << "*" << note << "(" << m_params[1] << " + " << m_params[2] << ") = " << m_params[0] << "; " << m_params[1] << " += " << m_params[2];
            else if (m_order == 2)
                os << "*" << note << "(" << m_params[1] << ") = " << m_params[0] << "; " << m_params[1] << " += " << m_params[2];
            else os << "*" << note << "(" << m_params[1] << " + " << m_params[2] << ") = " << m_params[0];
        }
        else if (this->m_params.size() == 5) {
            const auto&
                a = m_params[0],
                b = m_params[1],
                c = m_params[2],
                d = m_params[4];
            if (m_params[3] == "ROR") {
                if (m_order == 1)
                    os << "*" << note << "(" << b << " + std::rotr(" << c << ", " << d << "))" << a << "; " << b << " += std::rotr(" << c << ", " << d << ")";
                else if (m_order == 2)
                    os << "*" << note << "(" << b << ") = " << a << "; " << b << " += std::rotr(" << c << ", " << d << ")";
                else os << "*" << note << "(" << b << " + std::rotr(" << c << ", " << d << ")) = " << a;
            }
            else if (m_params[3] == "ASR" || m_params[3] == "LSR") {
                if (m_order == 1)
                    os << "*" << note << "(" << b << " + (" << c << " >> " << d << ")) = " << a << "; " << b << " += (" << c << " >> " << d << ")";
                else if (m_order == 2)
                    os << "*" << note << "(" << b << ") = " << a << "; " << b << " += (" << c << " >> " << d << ")";
                else os << "*" << note << "(" << b << " + (" << c << " >> " << d << ")) = " << a;
            }
            else if (m_params[3] == "LSL") {
                if (m_order == 1)
                    os << "*" << note << "(" << b << " + (" << c << " << " << d << ")) = " << a << "; " << b << " += (" << c << " << " << d << ")";
                else if (m_order == 2)
                    os << "*" << note << "(" << b << ") = " << a << "; " << b << " += (" << c << " << " << d << ")";
                else os << "*" << note << "(" << b << " + (" << c << " << " << d << ")) = " << a;
            }
            else goto param_error;
        }
        else goto param_error;
    } break;
    case command_t::SUB:
        // SUB{S}{cond} {Rd}, Rn, Operand2
        if (this->m_params.size() == 2)
            os << m_params[0] << " -= " << m_params[1];
        else if (this->m_params.size() == 3)
            os << m_params[0] << " = " << m_params[1] << " - " << m_params[2];
        else goto param_error;
        break;
    case command_t::TEQ:
        // TEQ{cond} Rn, Operand2
        if (this->m_params.size() == 2)
            os << "if ((" << m_params[0] << " ^ " << m_params[1] << ") ? X) ...";
        else goto param_error;
        break;
    case command_t::TST:
        // TST{cond} Rn, Operand2
        if (this->m_params.size() == 2)
            os << "if ((" << m_params[0] << " & " << m_params[1] << ") ? X) ...";
        else goto param_error;
        break;
    case command_t::UBFX:
        // UBFX{cond} Rd, Rn, #lsb, #width
        if (this->m_params.size() == 4)
            os << m_params[0] << " = ((unsigned)" << m_params[1] << " & ~(bitfield(" << m_params[3] << ") << " << m_params[2] << ")";
        else goto param_error;
        break;
    case command_t::UDIV:
        // UDIV{cond} {Rd}, Rn, Rm
        if (this->m_params.size() == 2)
            os << m_params[0] << " /= (unsigned)" << m_params[1];
        else if (this->m_params.size() == 3)
            os << m_params[0] << " = (unsigned)" << m_params[1] << " / (unsigned)" << m_params[2];
        else goto param_error;
        break;
    case command_t::UMAAL:
        // UMAAL{cond} RdLo, RdHi, Rn, Rm
        if (this->m_params.size() == 4)
            os << m_params[0] << m_params[1] << " = (" << m_params[2] << " * " << m_params[3] << ") + " << m_params[0] << " + " << m_params[1];
        else goto param_error;
        break;
    case command_t::UMLAL:
        // UMLAL{S}{cond} RdLo, RdHi, Rn, Rm
        if (this->m_params.size() == 4)
            os << m_params[0] << m_params[1] << " = (" << m_params[2] << " * " << m_params[3] << ") + ((" << m_params[0] << " << 32) | " << m_params[1] << ")";
        else goto param_error;
        break;
    case command_t::UMULL:
        // UMULL{S}{cond} RdLo, RdHi, Rn, Rm
        if (this->m_params.size() == 4)
            os << m_params[0] << m_params[1] << " = (" << m_params[2] << " * " << m_params[3] << ")";
        else goto param_error;
        break;
    case command_t::UXTB:
        // UXTB{cond} {Rd}, Rm {,rotation}
        if (this->m_params.size() == 1)
            os << m_params[0] << " &= 0xFF";
        else if (this->m_params.size() == 2)
            os << m_params[0] << " = " << m_params[1] << " & 0xFF";
        else if (this->m_params.size() == 3 && this->m_params[1] == "ROR")
            os << m_params[0] << " = std::rotr(" << m_params[0] << ", " << m_params[2] << ") & 0xFF";
        else if (this->m_params.size() == 4 && this->m_params[2] == "ROR")
            os << m_params[0] << " = std::rotr(" << m_params[1] << ", " << m_params[3] << ") & 0xFF";
        break;
        break;
    case command_t::UXTH:
        // UXTH{cond} {Rd}, Rm {,rotation}
        if (this->m_params.size() == 1)
            os << m_params[0] << " &= 0xFFFF";
        else if (this->m_params.size() == 2)
            os << m_params[0] << " = " << m_params[1] << " & 0xFFFF";
        else if (this->m_params.size() == 3 && this->m_params[1] == "ROR")
            os << m_params[0] << " = std::rotr(" << m_params[0] << ", " << m_params[2] << ") & 0xFFFF";
        else if (this->m_params.size() == 4 && this->m_params[2] == "ROR")
            os << m_params[0] << " = std::rotr(" << m_params[1] << ", " << m_params[3] << ") & 0xFFFF";
        break;
    default: os << "asm(\"" << this->m_original << "\")"; break;
    }

    if (this->m_comments.size() > 0)
        os << "; // " << this->m_comments;
    else os << ";";
    return os.str();

 param_error:
    throw std::runtime_error("illegal number of args");
}

// https://developer.arm.com/documentation/dui0379/f/arm-and-thumb-instructions/adc?lang=en
// ADC{S}{cond} {Rd}, Rn, Operand2
// ADD{S}{cond} {Rd}, Rn, Operand2
// AND{S}{cond} Rd, Rn, Operand2
// ASR{S}{cond} Rd, Rm, Rs
// B{cond} label
// BFC{cond} Rd, #lsb, #width
// BFI{cond} Rd, Rn, #lsb, #width
// BIC{S}{cond} Rd, Rn, Operand2
// BKPT #imm
// BL{cond} label
// BLX{cond} label
// BLX{cond} Rm
// BX{cond} Rm
// CBZ Rn, label
// CBNZ Rn, label
// CLZ{cond} Rd, Rm
// CMP{cond} Rn, Operand2
// CMN{cond} Rn, Operand2
// EOR{S}{cond} Rd, Rn, Operand2
// LDM{addr_mode}{cond} Rn{!}, reglist{^}
// LDR{type}{cond} Rt, [Rn {, #offset}]
// LDR{type}{cond} Rt, [Rn, #offset]!
// LDR{type}{cond} Rt, [Rn], #offset
// LDR{type}{cond} Rt, [Rn, ±Rm {, shift}]
// LDR{type}{cond} Rt, [Rn, ±Rm {, shift}]!
// LDR{type}{cond} Rt, [Rn], ±Rm {, shift}
// LSL{S}{cond} Rd, Rm, Rs
// LSR{S}{cond} Rd, Rm, Rs
// MLA{S}{cond} Rd, Rn, Rm, Ra
// MLS{cond} Rd, Rn, Rm, Ra
// MOV{S}{cond} Rd, Operand2
// MOVT{cond} Rd, #imm16
// MUL{S}{cond} {Rd}, Rn, Rm
// MVN{S}{cond} Rd, Operand2
// NOP{cond}
// ORR{S}{cond} Rd, Rn, Operand2
// POP{cond} reglist
// PUSH{cond} reglist
// ROR{S}{cond} Rd, Rm, Rs
// RSB{S}{cond} {Rd}, Rn, Operand2
// RSC{S}{cond} {Rd}, Rn, Operand2
// SBC{S}{cond} {Rd}, Rn, Operand2
// SBFX{cond} Rd, Rn, #lsb, #width
// SDIV{cond} {Rd}, Rn, Rm
// STM{addr_mode}{cond} Rn{!}, reglist{^}
// STR{type}{cond} Rt, [Rn {, #offset}]
// STR{type}{cond} Rt, [Rn, #offset]!
// STR{type}{cond} Rt, [Rn], #offset
// STR{type}{cond} Rt, [Rn, ±Rm {, shift}]
// STR{type}{cond} Rt, [Rn, ±Rm {, shift}]!
// STR{type}{cond} Rt, [Rn], ±Rm {, shift}
// SUB{S}{cond} {Rd}, Rn, Operand2
// TEQ{cond} Rn, Operand2
// TST{cond} Rn, Operand2
// UBFX{cond} Rd, Rn, #lsb, #width
// UDIV{cond} {Rd}, Rn, Rm
// UMAAL{cond} RdLo, RdHi, Rn, Rm
// UMLAL{S}{cond} RdLo, RdHi, Rn, Rm
// UMULL{S}{cond} RdLo, RdHi, Rn, Rm
// UXTB{cond} {Rd}, Rm {,rotation}
// UXTH{cond} {Rd}, Rm {,rotation}
