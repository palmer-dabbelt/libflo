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
        
    public:
        /* Writes this node to a file with a newline at the end. */
        void writeln(FILE *f);

    public:
        /* Parses a node, given a string that represents that node. */
        static node_ptr parse(const std::string &line);
    };
}

#endif
