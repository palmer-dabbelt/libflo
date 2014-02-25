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
#include "opcode.h++"
#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace libflo;

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

node::node(const std::string d,
           const std::string op)
    : _d(d),
      _altwidth(-1),
      _op(op),
      _s()
{
}

node::node(const std::string d,
           const std::string op,
           const std::string s0)
    : _d(d),
      _altwidth(-1),
      _op(op),
      _s({s0})
{
}

node::node(const std::string d,
           const std::string op,
           const std::string s0,
           const std::string s1)
    : _d(d),
      _altwidth(-1),
      _op(op),
      _s({s0, s1})
{
}

node::node(const std::string d,
           const std::string op,
           const std::string s0,
           const std::string s1,
           const std::string s2)
    : _d(d),
      _altwidth(-1),
      _op(op),
      _s({s0, s1, s2})
{
}

node::node(const std::string d,
           opwidthp op,
           const unsigned altwidth,
           const std::vector<std::string> s)
    : _d(d),
      _altwidth(altwidth),
      _op(op),
      _s(s)
{
}

void node::writeln(FILE *f)
{
    fprintf(f, "%s = %s", _d.c_str(), _op.to_string().c_str());
    for (auto it = _s.begin(); it != _s.end(); ++it)
        fprintf(f, " %s", (*it).c_str());
    fprintf(f, "\n");
}

const std::string node::to_string(void) const
{
    char buffer[LINE_MAX];
    switch (_s.size()) {
    case 0:
        snprintf(buffer, LINE_MAX, "%s = %s",
                 _d.c_str(),
                 _op.to_string().c_str()
            );
        break;

    case 1:
        snprintf(buffer, LINE_MAX, "%s = %s %s",
                 _d.c_str(),
                 _op.to_string().c_str(),
                 _s[0].c_str()
            );
        break;

    case 2:
        snprintf(buffer, LINE_MAX, "%s = %s %s %s",
                 _d.c_str(),
                 _op.to_string().c_str(),
                 _s[0].c_str(),
                 _s[1].c_str()
            );
        break;

    case 3:
        snprintf(buffer, LINE_MAX, "%s = %s %s %s %s",
                 _d.c_str(),
                 _op.to_string().c_str(),
                 _s[0].c_str(),
                 _s[1].c_str(),
                 _s[2].c_str()
            );
        break;

    default:
        fprintf(stderr, "Too many arguments to node\n");
        abort();
    }

    return buffer;
}

node_ptr node::with_width(unsigned width) const
{
    return node_ptr(new node(_d,
                             opwidthp(_op.opcode(), width),
                             _altwidth,
                             _s)
        );
}

node_ptr node::with_alt_width(unsigned width) const
{
    return node_ptr(new node(_d,
                             _op,
                             width,
                             _s)
        );
}

unsigned node::width(void) const
{
    switch (this->opcode()) {
    case libflo::opcode::EQ:
    case libflo::opcode::GTE:
    case libflo::opcode::LT:
    case libflo::opcode::NEQ:
    case libflo::opcode::ADD:
    case libflo::opcode::AND:
    case libflo::opcode::ARSH:
    case libflo::opcode::EAT:
    case libflo::opcode::IN:
    case libflo::opcode::LD:
    case libflo::opcode::LIT:
    case libflo::opcode::LOG2:
    case libflo::opcode::LSH:
    case libflo::opcode::MEM:
    case libflo::opcode::MOV:
    case libflo::opcode::MUL:
    case libflo::opcode::MUX:
    case libflo::opcode::MSK:
    case libflo::opcode::NEG:
    case libflo::opcode::NOP:
    case libflo::opcode::NOT:
    case libflo::opcode::OR:
    case libflo::opcode::OUT:
    case libflo::opcode::RD:
    case libflo::opcode::REG:
    case libflo::opcode::RND:
    case libflo::opcode::RST:
    case libflo::opcode::ST:
    case libflo::opcode::SUB:
    case libflo::opcode::WR:
    case libflo::opcode::XOR:
    case libflo::opcode::CAT:
        return _op.width();

    case libflo::opcode::RSH:
        if (_altwidth == (unsigned)-1)
            return _op.width();
        return _altwidth;
    }

    fprintf(stderr, "Made it past an opcode switch\n");
    abort();
}

unsigned node::outwid(void) const
{
    switch (this->opcode()) {
    case libflo::opcode::EQ:
    case libflo::opcode::GTE:
    case libflo::opcode::LT:
    case libflo::opcode::NEQ:
        return 1;

    case libflo::opcode::ADD:
    case libflo::opcode::AND:
    case libflo::opcode::ARSH:
    case libflo::opcode::EAT:
    case libflo::opcode::IN:
    case libflo::opcode::LD:
    case libflo::opcode::LIT:
    case libflo::opcode::LOG2:
    case libflo::opcode::LSH:
    case libflo::opcode::MEM:
    case libflo::opcode::MOV:
    case libflo::opcode::MUL:
    case libflo::opcode::MUX:
    case libflo::opcode::MSK:
    case libflo::opcode::NEG:
    case libflo::opcode::NOP:
    case libflo::opcode::NOT:
    case libflo::opcode::OR:
    case libflo::opcode::OUT:
    case libflo::opcode::RD:
    case libflo::opcode::REG:
    case libflo::opcode::RND:
    case libflo::opcode::RST:
    case libflo::opcode::RSH:
    case libflo::opcode::ST:
    case libflo::opcode::SUB:
    case libflo::opcode::WR:
    case libflo::opcode::XOR:
        return _op.width();

    case libflo::opcode::CAT:
        return _altwidth;
    }

    fprintf(stderr, "Made it past an opcode switch\n");
    abort();
}

node_ptr node::parse(const std::string &line)
{
    /* Here we just use sscanf to parse the line because the format is
     * pretty fixed. */
    char d[LINE_MAX], s0[LINE_MAX], s1[LINE_MAX], s2[LINE_MAX];
    char op[LINE_MAX];
    int scanned = sscanf(line.c_str(), "%s = %s %s %s %s\n",
                         d, op, s0, s1, s2);

    switch (scanned) {
    case 5:
        return node_ptr(new node(d, op, s0, s1, s2));
    case 4:
        return node_ptr(new node(d, op, s0, s1));
    case 3:
        return node_ptr(new node(d, op, s0));
    case 2:
        return node_ptr(new node(d, op));
    }

    fprintf(stderr, "Unable to parse '%s' as a flo node\n",
            line.c_str());
    fprintf(stderr, "  parsed %d values\n", scanned);
    abort();
}
