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

#include "node.h++"
#include "globals.h++"
#include "sizet_printf.h++"
#include <string>
#include <string.h>
using namespace libflo;

#ifndef LINE_SIZE
#define LINE_SIZE 1024
#endif

node::node(const std::string name,
           const unknown<size_t>& width,
           const unknown<size_t>& depth,
           bool is_mem,
           bool is_const,
           unknown<size_t> dfdepth,
           const unknown<size_t>& x,
           const unknown<size_t>& y)
    : _name(str2name(name)),
      _width(str2width(name, width)),
      _depth(depth),
      _is_mem(is_mem),
      _is_const(is_const),
      _dfdepth(dfdepth),
      _x(x),
      _y(y)
{
}

void node::update_width(const unknown<size_t>& width)
{
    /* Checks that if we do overwrite a width that it doesn't change
     * in the process. */
    if (_width.known() && width.known()) {
        if (_width.value() != width.value()) {
            fprintf(stderr, "Re-inferring node width: '%s'\n", name().c_str());
            fprintf(stderr, "  Old width: " SIZET_FORMAT "\n", _width.value());
            fprintf(stderr, "  New width: " SIZET_FORMAT "\n", width.value());
            if (globals::allow_bad_widths == false)
                abort();
        }
    }

    if (width.known())
        _width = width;
}

void node::update_dfdepth(const unknown<size_t>& dfdepth)
{
    /* Checks that if we do overwrite a dfdepth that it doesn't change
     * in the process. */
    if (_dfdepth.known() && dfdepth.known()) {
        if (_dfdepth.value() != dfdepth.value()) {
            fprintf(stderr, "Re-scheduling node dfdepth\n");
            abort();
        }
    }

    if (dfdepth.known())
        _dfdepth = dfdepth;
}

int node::const_int(void) const
{
    if (!is_const()) {
        fprintf(stderr, "const_int taken for non-constant\n");
        abort();
    }

    return atoi(name().c_str());
}

std::string node::str2name(const std::string n)
{
    char buffer[LINE_SIZE];
    snprintf(buffer, LINE_SIZE, "%s", n.c_str());

    for (size_t i = 0; i < strlen(buffer); ++i) {
        if (buffer[i] == '\'' || buffer[i] == '/')
            buffer[i] = '\0';
    }

    return buffer;
}

unknown<size_t> node::str2width(const std::string n, const unknown<size_t>& w)
{
    char buffer[LINE_SIZE];
    snprintf(buffer, LINE_SIZE, "%s", n.c_str());

    char *width = strstr(buffer, "/");
    if (width == NULL)
        width = strstr(buffer, "'");
    if (width == NULL)
        return w;
    width++;

    unknown<size_t> nw(atoll(width));

    if (w.known() && (nw.value() != w.value())) {
        fprintf(stderr, "Mismatched width parsing node; '%s'\n", n.c_str());
        fprintf(stderr, "  Parsed width: " SIZET_FORMAT "\n", nw.value());
        fprintf(stderr, "  Passed width: " SIZET_FORMAT "\n", w.value());
    }

    return nw;
}

unknown<size_t> node::str2width(const std::string n)
{
    return str2width(n, unknown<size_t>());
}
