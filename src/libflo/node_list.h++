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

#ifndef LIBFLO__NODE_LIST_HXX
#define LIBFLO__NODE_LIST_HXX

#include "node.h++"
#include <vector>

namespace libflo {
    /* This simply holds a list of nodes. */
    class node_list {
    public:
        /* Iterates over the set of nodes in this list. */
        class iter {
        private:
            std::vector<node_ptr> _nodes;
            std::vector<node_ptr>::const_iterator _it;

        public:
            iter(const std::vector<node_ptr> &nodes)
                : _nodes(nodes),
                  _it(_nodes.begin())
                {
                }

            node_ptr operator*(void) const { return *_it; }
            void operator++(void) { ++_it; }
            bool done(void) const { return _it == _nodes.end(); }
        };

    private:
        std::vector<node_ptr> _nodes;

    public:
        /* Creates a new, empty, node list. */
        node_list(void);

        /* Adds a node to this list, in an unspecified order. */
        node_ptr add(node_ptr a);

        /* Returns an iterator that allows us to walk this list. */
        iter nodes(void) const;
    };
}

#endif
