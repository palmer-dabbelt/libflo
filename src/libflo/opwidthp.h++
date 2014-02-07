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

#ifndef LIBFLO__OPWIDTHP_HXX
#define LIBFLO__OPWIDTHP_HXX

#include "opcode.h++"
#include <string>

namespace libflo {
    /* Represents an operation along with an (optional) width.  The
     * idea here is that some operations that come out of the Flo
     * backend are tagged with a width while others aren't.  The goal
     * of this class is to provide a type-safe way of saying that
     * these operations may not have widths: use the opwidth class if
     * you _know_ that there's always a width attached. */
    class opwidthp {
    private:
        const bool _has_width;
        const unsigned _width;
        const enum opcode _opcode;

    public:
        opwidthp(const std::string op);
        opwidthp(const opwidthp &op);
        opwidthp(enum opcode opcode, unsigned width);

        bool has_width(void) const { return _has_width; }
        unsigned width(void) const { if (!_has_width) abort(); return _width; }
        enum opcode opcode(void) const { return _opcode; }

        const std::string to_string(void) const;
    };
}

#endif
