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

#ifndef LIBFLO__NODE_HXX
#define LIBFLO__NODE_HXX

#include "opwidthp.h++"
#include <memory>
#include <string>
#include <vector>

namespace libflo {
    /* Nodes are all reference-counted, all the time. */
    class node;
    typedef std::shared_ptr<node> node_ptr;

    /* This simply holds a list of nodes. */
    class node {
    private:
        const std::string _d;
        const opwidthp _op;
        const std::vector<std::string> _s;

    private:
        /* These various constructors just fill in different parts of
         * the node.  The idea here is to allow the parse() function
         * to act a bit like a constructor, but one that can maintain
         * const safety. */
        node(const std::string d,
             const std::string op);
        node(const std::string d,
             const std::string op,
             const std::string s0);
        node(const std::string d,
             const std::string op,
             const std::string s0,
             const std::string s1);
        node(const std::string d,
             const std::string op,
             const std::string s0,
             const std::string s1,
             const std::string s2);

    protected:
        /* Effectively a copy constructor, but you can replace some
         * parts. */
        node(const std::string d,
             opwidthp op,
             const std::vector<std::string> s);
        
    public:
        /* Accessor functions. */
        const std::string d(void) const { return _d; }
        const opwidthp op(void) const { return _op; }
        const std::vector<std::string> s(void) const { return _s; }

        const std::vector<std::string>::const_iterator s_begin(void)
            { return _s.begin(); }
        const std::vector<std::string>::const_iterator s_end(void)
            { return _s.end(); }

        const std::string s(size_t i) { return _s[i]; }
        const std::string o(size_t i) { return (i == 0) ? _d : _s[i-1]; }

        enum opcode opcode(void) const { return _op.opcode(); }
        unsigned width(void) const { return _op.width(); }

        /* Writes this node to a file with a newline at the end. */
        void writeln(FILE *f);

        /* Returns a new node with the width parameter set. */
        node_ptr with_width(unsigned width) const;

        /* Returns the width of the output of this node.  Note that
         * this is different than the "width", which is the operation
         * width.  For example "eq/32" has an output width of 1 but an
         * operation width of 32, which means it compares 32 bits and
         * returns one. */
        unsigned outwid(void) const;

    public:
        /* Parses a node, given a string that represents that node. */
        static node_ptr parse(const std::string &line);
    };
}

#endif
