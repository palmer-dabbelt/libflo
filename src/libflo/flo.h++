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

#ifndef LIBFLO__FLO_HXX
#define LIBFLO__FLO_HXX

#include "node.h++"
#include "operation.h++"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace libflo {
    /* Stores a single Flo object: this holds all the information
     * about a dataflow computation that's necessary to compute it.
     * This includes the node as well as every operation that's
     * required to make it work. */
    class flo {
        typedef std::shared_ptr<operation<node>> operation_ptr;

    public:
        class op_iter {
        private:
            const std::vector<operation_ptr> _ops;
            std::vector<operation_ptr>::const_iterator _it;

        public:
            op_iter(const std::vector<operation_ptr> ops)
                : _ops(ops),
                  _it(_ops.begin())
                {
                }

            operation_ptr operator*(void) const { return *_it; }
            bool done(void) const { return _it == _ops.end(); }
            void operator++(void) { ++_it; }
        };

    private:
        const std::map<std::string, std::shared_ptr<node>> _nodes;
        const std::vector<operation_ptr> _ops;

    protected:
        /* Creates a Flo store given a set of node elements and a
         * list of operations to be performed. */
        flo(const std::map<std::string, std::shared_ptr<node>>& nodes,
            const std::vector<operation_ptr>& ops);

    public:
        /* Parses the given file as a Flo file. */
        static const flo parse(const std::string filename);

        /* Iterates through this program, either by nodes or by
         * operations. */
        op_iter operations(void) const { return op_iter(_ops); }
    };
}

#endif
