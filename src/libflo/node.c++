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
#include <string>
using namespace libflo;

node::node(const std::string name,
           const unknown<size_t>& width,
           const unknown<size_t>& depth,
           bool is_mem,
           bool is_const,
           unknown<size_t> cycle)
    : _name(name),
      _width(width),
      _depth(depth),
      _is_mem(is_mem),
      _is_const(is_const),
      _cycle(cycle)
{
}

void node::update_width(const unknown<size_t>& width)
{
    /* Checks that if we do overwrite a width that it doesn't change
     * in the process. */
    if (_width.known() && width.known()) {
        if (_width.value() != width.value()) {
            fprintf(stderr, "Re-inferring node width\n");
            abort();
        }
    }

    if (width.known())
        _width = width;
}

void node::update_cycle(const unknown<size_t>& cycle)
{
    /* Checks that if we do overwrite a cycle that it doesn't change
     * in the process. */
    if (_cycle.known() && cycle.known()) {
        if (_cycle.value() != cycle.value()) {
            fprintf(stderr, "Re-scheduling node cycle\n");
            abort();
        }
    }

    if (cycle.known())
        _cycle = cycle;
}
