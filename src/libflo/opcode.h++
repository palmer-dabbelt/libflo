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

#include <string>

namespace libflo {
    /* This represents the operations that a DREAMER can perform. */
    enum class opcode {
        EAT,
        RND,
        MUX,
        SUB,
        AND,
        LT,
        NOT,
        REG,
        OR,
        EQ,
        ADD,
        RST,
        IN,
        LIT,
        CAT,
        RSH,
        MSK,
        LD,
        NEQ,
        ARSH,
        LSH,
        XOR,
        ST,
        MEM,
        OUT,
        GTE,
        MOV
    };

    /* Converts an opcode to a string, or parses a string into an
     * opcode. */
    const std::string&& opcode_to_string(enum opcode opcode);
    enum opcode string_to_opcode(const std::string string);
}

#endif
