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

#include "filenode.h++"
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
        typedef std::shared_ptr<node> node_ptr;

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
        const std::map<std::string, node_ptr> _nodes;
        const std::vector<operation_ptr> _ops;

    protected:
        /* Creates a Flo store given a set of node elements and a
         * list of operations to be performed. */
        flo(const std::map<std::string, node_ptr>& nodes,
            const std::vector<operation_ptr>& ops)
            : _nodes(nodes),
              _ops(ops)
            {
            }

    public:
        /* Iterates through this program, either by nodes or by
         * operations. */
        op_iter operations(void) const { return op_iter(_ops); }

    public:
        /* Parses the given file as a Flo file. */
        static const flo parse(const std::string filename)
            {
                /* Reads a file line by line, saving it into memory. */
                std::vector<filenode> lines;
                {
                    FILE *f = fopen(filename.c_str(), "r");
                    if (f == NULL) {
                        fprintf(stderr, "Unable to open '%s'\n",
                                filename.c_str());
                        abort();
                    }

                    char buffer[LINE_MAX];
                    while (fgets(buffer, LINE_MAX, f) != NULL) 
                        lines.push_back(filenode::parse(buffer));
                }

                /* The first thing we need to do is produce a
                 * hash-table of all the node elements -- this is used
                 * later when looking up these elements.  Essentially
                 * this keeps all the string lookups inside here. */
                std::map<std::string, node_ptr> nodes;
                for (auto it = lines.begin(); it != lines.end(); ++it) {
                    auto line = *it;
                    auto node = node::parse(line.d,
                                            line.opcode,
                                            line.width,
                                            line.s);
                    if (node != NULL) {
                        nodes[node->name()] = node;
                    }
                }

                /* Now we need to produce a map of operations, which
                 * consist of the actual computation.  In addition to
                 * actually producing the operation structures, here
                 * we handle generating nodes for every constant. */
                std::vector<operation_ptr> ops;
                for (auto it = lines.begin(); it != lines.end(); ++it) {
                    auto line = *it;
                    auto op = operation<node>::parse(nodes,
                                                     line.d,
                                                     line.opcode,
                                                     line.width,
                                                     line.s);
                    if (op != NULL)
                        ops.push_back(op);
                }

                /* At this point we've got the operation list, but we
                 * still need to perform width inference. */
                for (size_t i = 0; i < nodes.size(); ++i) {
                    for (auto it = ops.begin(); it != ops.end(); ++it) {
                        auto op = *it;
                        op->try_infer_width();
                    }

                    /* Check to see how many unknown widths still
                     * remain -- if there's none then we're done with
                     * this phase and can continue, otherwise attempt
                     * to keep going. */
                    size_t remaining_unknowns = 0;
                    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
                        auto node = it->second;
                        if (!node->known_width())
                            remaining_unknowns++;
                    }
                    if (remaining_unknowns == 0)
                        break;
                }

                /* If we've made it this far then hopefully we know
                 * the width of every operation -- if we don't then
                 * bail out. */
                {
                    size_t remaining_unknows = 0;
                    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
                        auto node = it->second;
                        if (!node->known_width()) {
                            fprintf(stderr,
                                    "Unable to infer width for node '%s'\n",
                                    node->name().c_str());
                            remaining_unknows++;
                        }
                    }

                    if (remaining_unknows > 0) {
                        fprintf(stderr, "Aborting with %lu widths remaining\n",
                                remaining_unknows);
                        abort();
                    }
                }

                /* Make an attempt to schedule this computation in
                 * dataflow order to something can actually be
                 * computed using this program. */
                for (size_t i = 0; i < nodes.size(); ++i) {
                    for (auto it = ops.begin(); it != ops.end(); ++it) {
                        auto op = *it;
                        op->try_schedule();
                    }

                    /* Check to see how many unknown widths still
                     * remain -- if there's none then we're done with
                     * this phase and can continue, otherwise attempt
                     * to keep going. */
                    size_t remaining_unknowns = 0;
                    for (auto it = ops.begin(); it != ops.end(); ++it) {
                        auto op = *it;
                        if (!op->known_width())
                            remaining_unknowns++;
                    }
                    if (remaining_unknowns == 0)
                        break;
                }

                /* If we've made it this far then hopefully we've
                 * scheduled every operation -- if we don't then bail
                 * out. */
                {
                    size_t remaining_unknows = 0;
                    for (auto it = ops.begin(); it != ops.end(); ++it) {
                        auto op = *it;
                        if (!op->known_cycle()) {
                            fprintf(stderr, "Unable to schedule node '%s'\n",
                                    op->d()->name().c_str());
                            remaining_unknows++;
                        }
                    }

                    if (remaining_unknows > 0) {
                        fprintf(stderr, "Aborting with %lu nodes remaining\n",
                                remaining_unknows);
                        abort();
                    }
                }

                return flo(nodes, ops);
            }
    };
}

#endif
