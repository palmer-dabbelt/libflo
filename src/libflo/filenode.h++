/*
 * Copyright (C) 2014 Palmer Dabbelt
 *   <palmer.dabbelt@eecs.berkeley.edu>
 *
 * This file is part of libflo.
 *
 * libflo is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libflo is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libflo.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef LIBFLO__FILENODE_HXX
#define LIBFLO__FILENODE_HXX

#include "opcode.h++"
#include "unknown.h++"
#include <memory>
#include <string>
#include <vector>

namespace libflo {
    /* Holds the result from reading and tokenizing an input file.
     * This expects a single line to come in a Flo file, and
     * essentially just stores it for later.  This is only used
     * interally to the parser and shouldn't be used anywhere else. */
    class filenode {
    public:
        const std::string d;
        const libflo::opcode opcode;
        const unknown<size_t> width;
        const std::vector<std::string> s;

    private:
        filenode(const std::string d, const std::string op_width,
                 const std::vector<std::string> s);
        filenode(const std::string d, const std::string op_width);

    public:
        static std::shared_ptr<filenode> parse(const std::string line);
    };
}

#endif
