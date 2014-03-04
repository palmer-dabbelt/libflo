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

#ifndef LIBFLO__OPERATION_HXX
#define LIBFLO__OPERATION_HXX

#include "node.h++"
#include "unknown.h++"
#include <map>
#include <memory>
#include <vector>

namespace libflo {
    /* Operations represent the computation of a node -- this means
     * they take as their input the values of some nodes and produce
     * as an output the values of other nodes. */
    class operation {
    private:
        std::shared_ptr<node> _d;
        unknown<size_t> _width;
        const opcode _op;
        const std::vector<std::shared_ptr<node>> _s;

    private:
        operation(std::shared_ptr<node>& dest,
                  const unknown<size_t>& width,
                  const opcode& op,
                  const std::vector<std::shared_ptr<node>>& s);

    public:
        /* Accessor functions. */
        size_t width(void) const { return _width.value(); }
        bool known_width(void) const { return _width.known(); }

        size_t cycle(void) const { return _d->cycle(); }
        bool known_cycle(void) const { return _d->known_cycle(); }

        /* Allows access to the destination, the source array, or the
         * operand array -- this just contains two indexing schemes
         * for the nodes this operation can see. */
        const std::shared_ptr<node> d(void) const { return _d; }
        const std::shared_ptr<node> s(size_t i) const { return _s[i]; }
        const std::shared_ptr<node> o(size_t i) const
            { return (i == 0) ? d() : s(i-1); }

        /* Attempts to perform local width inference.  In other words,
         * infers the width of this node by only looking at the nodes
         * directly used by this operation. */
        void try_infer_width(void);

        /* Attempts to perform local scheduling -- note that this is
         * just a simple schedule, it produces the first cycle at
         * which this operation could possibly run if dataflow order
         * is observed.. */
        void try_schedule(void);

        /* Writes this operation out to standard out as a string. */
        void writeln(FILE *f) const;

    private:
        /* Here's some helper functions for width inference.  They
         * shouldn't be useful to anyone else. */
        void must_match(const std::vector<size_t>& o);
        void must_sum(size_t o, const std::vector<size_t>& i);
        void must_be(size_t o, size_t w);

        /* Used for scheduling. */
        void after(const std::vector<size_t>& o);

    public:
        /* Parses an operation, looking up the sources and
         * destinations by string in the provided map. */
        static std::shared_ptr<operation>
        parse(const std::map<std::string, std::shared_ptr<node>>& n,
              const std::string d,
              const opcode& op,
              const unknown<size_t>& width,
              const std::vector<std::string>& s);
    };
}

#endif
