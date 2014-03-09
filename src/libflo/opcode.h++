/*
 * Copyright (C) 2014 Palmer Dabbelt
 *   <palmer.dabbelt@eecs.berkeley.edu>
 *
 * This file is part of DREAMER.
 *
 * DREAMER is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * DREAMER is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with DREAMER.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef LIBFLO__OPCODE_HXX
#define LIBFLO__OPCODE_HXX

#include "unknown.h++"
#include <string>

namespace libflo {
    /* This represents the operations that a DREAMER can perform. */
    enum class opcode {
        ADD,
        AND,
        ARSH,
        CAT,
        CATD,
        EAT,
        EQ,
        GTE,
        IN,
        LD,
        LIT,
        LOG2,
        LSH,
        LT,
        MEM,
        MOV,
        MSK,
        MUL,
        MUX,
        NEG,
        NEQ,
        NOP,
        NOT,
        OR,
        OUT,
        RD,
        REG,
        RND,
        RSH,
        RST,
        ST,
        SUB,
        WR,
        XOR,
    };

    /* Converts an opcode to a string, or parses a string into an
     * opcode. */
    const std::string&& opcode_to_string(enum opcode opcode);
    enum opcode string_to_opcode(const std::string string);

    /* Obtains the width that may or may not be attached to an
     * operation string. */
    const unknown<size_t> opstring_to_width(const std::string string);
}

#endif
