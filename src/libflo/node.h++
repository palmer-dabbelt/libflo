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

#include "opcode.h++"
#include "unknown.h++"
#include <memory>
#include <string>
#include <vector>

namespace libflo {
    /* Everything in the dataflow machine that can have a value is
     * considered a node. */
    class node {
    private:
        const std::string _name;
        unknown<size_t> _width;
        unknown<size_t> _depth;
        const bool _is_mem;
        const bool _is_const;
        unknown<size_t> _cycle;

    private:
        node(const std::string name,
             const unknown<size_t>& width,
             const unknown<size_t>& depth,
             bool is_mem,
             bool is_const,
             unknown<size_t> cycle);

    public:
        /* Accessor functions. */
        const std::string& name(void) const { return _name; }

        size_t width(void) const { return _width.value(); }
        bool known_width(void) const { return _width.known(); }

        size_t depth(void) const { return _depth.value(); }
        bool known_depth(void) const { return _depth.known(); }

        bool is_mem(void) const { return _is_mem; }
        bool is_reg(void) const { return !_is_mem; }
        bool is_const(void) const { return _is_const; }

        size_t cycle(void) const { return _cycle.value(); }
        bool known_cycle(void) const { return _cycle.known(); }

        /* Updates a node with a new width -- this will fail if both
         * widths are known and they don't match. */
        void update_width(const unknown<size_t>& width);

        /* Updates a node with a new cycle, failing on mismatch. */
        void update_cycle(const unknown<size_t>& cycle);

    public:
        /* Parses a node to determine exactly what sort of node it
         * is.  You probably don't want to bother with this unless
         * you're inside the parser... */
        static std::shared_ptr<node> parse(const std::string d,
                              const opcode& op,
                              const unknown<size_t>& width,
                              const std::vector<std::string>& s);

        /* Generates a new constant-valued node. */
        static std::shared_ptr<node> constant(int64_t value);
        static std::shared_ptr<node> reg(const std::string name,
                            const unknown<size_t>& width,
                            const unknown<size_t>& cycle);
        static std::shared_ptr<node> mem(const std::string name,
                            const unknown<size_t>& width,
                            const unknown<size_t>& depth);
    };
}

#endif
