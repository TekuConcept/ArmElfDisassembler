/**
 * Created by TekuConcept on March 24, 2020
 */

#ifndef ARM_INSTRUCTION_H
#define ARM_INSTRACTION_H

#include <string>
#include <vector>

namespace arm {
    // adc, add, and, asr, b, bfc, bfi, bic, bkpt,
    // bl, blx, bx, cbz, cbnz, clz, cmp, cmn, eor, ldm, ldr,
    // lsl, lsr, mla, mls, mov, movt, mul, mvn, nop, orr,
    // pop, push, ror, rsb, rsc, sbc, sbfx, sdiv, stm, str, sub,
    // teq, tst, ubfx, udiv, umaal, umlal, umull, uxtb, uxth

    enum class command_t {
        UNKNOWN, ADC, ADD/*, ADR*/, AND, ASR, B, BFC, BFI, BIC, BKPT,
        BL, BLX, BX, CBZ, CBNZ, CLZ, CMP, CMN, EOR, LDM, LDR,
        LSL, LSR, MLA, MLS, MOV, MOVT, MUL, MVN, NOP, ORR,
        POP, PUSH, ROR, RSB, RSC, SBC, SBFX, SDIV, STM, STR, SUB,
        TEQ, TST, UBFX, UDIV, UMAAL, UMLAL, UMULL, UXTB, UXTH
    };

    enum class condition_t {
        UNSPECIFIED,
        AL = 0, // Always (this is the default)
        EQ,     // Equal
        NE,     // Not equal
        CS,     // Carry set (identical to HS)
        HS,     // Unsigned higher or same (identical to CS)
        CC,     // Carry clear (identical to LO)
        LO,     // Unsigned lower (identical to CC)
        MI,     // Minus or negative result
        PL,     // Positive or zero result
        VS,     // Overflow
        VC,     // No overflow
        HI,     // Unsigned higher
        LS,     // Unsigned lower or same
        GE,     // Signed greater than or equal
        LT,     // Signed less than
        GT,     // Signed greater than
        LE,     // Signed less than or equal
    };

    enum class address_increment_order { IA = 0, IB, DA, DB };
    enum class register_data_size { W = 0, B, SB, H, SH, D };

    class instruction_t {
    public:
        static instruction_t parse(const std::string& line);

        inline command_t command() const
        { return m_command; }

        inline bool set() const
        { return m_set; }

        inline condition_t condition() const
        { return m_condition; }

        inline address_increment_order increment_order() const
        { return m_info.m_increment_order; }

        inline register_data_size data_size() const
        { return m_info.m_register_data_size; }

        inline unsigned int order() const
        { return m_order; }

        inline const std::vector<std::string>& parameters() const
        { return m_params; }

        inline const std::string& comments() const
        { return m_comments; }

        inline const std::string& to_string() const
        { return m_original; }

        std::string tag_string() const;

        std::string to_c() const;

    private:
        command_t m_command;
        bool m_set;
        condition_t m_condition;
        union {
            address_increment_order m_increment_order;
            register_data_size m_register_data_size;
        } m_info;
        unsigned int m_order;
        std::vector<std::string> m_params;
        std::string m_comments;
        std::string m_original;

        static command_t command_from_string(const std::string& s, size_t o = 0);
        static std::string command_to_string(command_t c);
        static size_t command_string_size(command_t c);
        static condition_t condition_from_chars(char a, char b);
        static std::string condition_to_string(condition_t c);
        static std::string string_from_condition(condition_t c);
    };

}

#endif
