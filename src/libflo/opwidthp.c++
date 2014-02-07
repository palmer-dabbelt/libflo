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

#include "opwidthp.h++"
#include <string.h>

using namespace libflo;

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

/* I'd love to stick these functions inside the constructor, but I
 * can't because of const safety.  Thanks, C++! */
static bool get_has_width(const std::string op);
static unsigned get_width(const std::string op);
static enum opcode get_opcode(const std::string op);

opwidthp::opwidthp(const std::string op)
    : _has_width(get_has_width(op)),
      _width(get_width(op)),
      _opcode(get_opcode(op))
{
}

opwidthp::opwidthp(const opwidthp &op)
    : _has_width(op._has_width),
      _width(op._width),
      _opcode(op._opcode)
{
}

opwidthp::opwidthp(enum opcode opcode, unsigned width)
    : _has_width(true),
      _width(width),
      _opcode(opcode)
{
}

bool get_has_width(const std::string op)
{
    char buffer[LINE_MAX];
    strncpy(buffer, op.c_str(), LINE_MAX);

    char *width = strstr(buffer, "/");
    if (width == NULL)
        return false;

    return true;
}

unsigned get_width(const std::string op)
{
    char buffer[LINE_MAX];
    strncpy(buffer, op.c_str(), LINE_MAX);

    char *width = strstr(buffer, "/");
    if (width == NULL)
        return -1;

    return atoi(width + 1);
}

enum opcode get_opcode(const std::string op)
{
    char buffer[LINE_MAX];
    strncpy(buffer, op.c_str(), LINE_MAX);

    char *width = strstr(buffer, "/");
    if (width != NULL)
        *width = '\0';

    return string_to_opcode(buffer);
}

const std::string opwidthp::to_string(void) const
{
    char buffer[LINE_MAX];

    if (_has_width) {
        snprintf(buffer, LINE_MAX, "%s/%u",
                 opcode_to_string(_opcode).c_str(),
                 _width);
    } else {
        snprintf(buffer, LINE_MAX, "%s",
                 opcode_to_string(_opcode).c_str());
    }

    return buffer;
}

