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
           const unknown<size_t>& width)
    : _name(name),
      _width(width),
      _depth(unknown<size_t>()),
      _is_mem(false),
      _is_const(false)
{
}

node::node(const std::string name,
           const unknown<size_t>& width,
           const unknown<size_t>& depth)
    : _name(name),
      _width(width),
      _depth(depth),
      _is_mem(true),
      _is_const(false)
{
}

node::node(int64_t value)
    : _name(std::to_string(value)),
      _width(unknown<size_t>()),
      _depth(unknown<size_t>()),
      _is_mem(false),
      _is_const(true)
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

node_ptr node::parse(const std::string d, const opcode& op,
                       const unknown<size_t>& width,
                       const std::vector<std::string>& s)
{
    switch (op) {
        /* These operations simply produce a single bit as an output
         * regardles sof their input widths. */
    case opcode::EQ:
    case opcode::GTE:
    case opcode::LT:
    case opcode::NEQ:
    case opcode::RST:
        return node_ptr(new node(d, unknown<size_t>(1)));

        /* These are "normal" nodes, which means their output width is
         * specified directly by the operation. */
    case opcode::ADD:
    case opcode::AND:
    case opcode::ARSH:
    case opcode::IN:
    case opcode::LD:
    case opcode::LIT:
    case opcode::LOG2:
    case opcode::LSH:
    case opcode::MOV:
    case opcode::MSK:
    case opcode::MUL:
    case opcode::MUX:
    case opcode::NEG:
    case opcode::NOT:
    case opcode::OR:
    case opcode::OUT:
    case opcode::RD:
    case opcode::REG:
    case opcode::RND:
    case opcode::RSH:
    case opcode::ST:
    case opcode::SUB:
    case opcode::WR:
    case opcode::XOR:
        return node_ptr(new node(d, width));
        break;

        /* Some operations don't have their output width specified by
         * the Flo file but instead must have this width somehow
         * inferred. */
    case opcode::CAT:
        return node_ptr(new node(d, unknown<size_t>()));

        /* Memories are special: they have a depth paramater as well
         * as a width parameter.  It's possible to specify the depth
         * explicitly, but there's also some notion of being able to
         * automatically detect the width. */
    case opcode::MEM:
    {
        if (s.size() == 0)
            return node_ptr(new node(d, width, unknown<size_t>()));

        long long depth = atoll(s[0].c_str());
        if (depth == -1)
            return node_ptr(new node(d, width, unknown<size_t>()));

        size_t sd = (size_t)depth;
        return node_ptr(new node(d, width, unknown<size_t>(sd)));

        break;
    }

        /* These operations don't actually produce a node. */
    case opcode::EAT:
    case opcode::NOP:
        return NULL;
        break;
    }

    fprintf(stderr, "Unhandled switch case\n");
    abort();
}

node_ptr node::constant(int64_t value)
{
    return node_ptr(new node(value));
}
