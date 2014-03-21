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

#include "filenode.h++"
using namespace libflo;

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

filenode::filenode(const std::string _d, const std::string op_width,
                   const std::vector<std::string> _s)
    : d(_d),
      opcode(string_to_opcode(op_width)),
      width(opstring_to_width(op_width)),
      s(_s)
{
}

filenode::filenode(const std::string d, const std::string op_width)
    : d(d),
      opcode(string_to_opcode(op_width)),
      width(opstring_to_width(op_width)),
      s()
{
}

filenode filenode::parse(const std::string line)
{
    /* Here we just use sscanf to parse the line because the format is
     * pretty fixed. */
    char d[LINE_MAX], s0[LINE_MAX], s1[LINE_MAX], s2[LINE_MAX], s3[LINE_MAX];
    char op[LINE_MAX];
    int scanned = sscanf(line.c_str(), "%s = %s %s %s %s %s\n",
                         d, op, s0, s1, s2, s3);

    switch (scanned) {
    case 6:
        return filenode(d, op, std::vector<std::string>({s0, s1, s2, s3}));
    case 5:
        return filenode(d, op, std::vector<std::string>({s0, s1, s2}));
    case 4:
        return filenode(d, op, std::vector<std::string>({s0, s1}));
    case 3:
        return filenode(d, op, std::vector<std::string>({s0}));
    case 2:
        return filenode(d, op);
    }

    fprintf(stderr, "Unable to parse '%s' as a flo node\n",
            line.c_str());
    fprintf(stderr, "  parsed %d values\n", scanned);
    abort();
}
