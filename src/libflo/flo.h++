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
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

namespace libflo {
    /* Stores a single Flo object: this holds all the information
     * about a dataflow computation that's necessary to compute it.
     * This includes the node as well as every operation that's
     * required to make it work. */
    template<class node_t, class operation_t> class flo {
        typedef std::shared_ptr<operation_t> operation_ptr;
        typedef std::shared_ptr<node_t> node_ptr;
        typedef std::function<node_ptr(const std::string name,
                                       const unknown<size_t>& width,
                                       const unknown<size_t>& depth,
                                       bool is_mem,
                                       bool is_const,
                                       unknown<size_t> dfdepth,
                                       const unknown<std::string>& posn
            )> node_create_func_t;

    private:
        std::map<std::string, node_ptr> _nodes;
        std::vector<node_ptr> _node_vec;
        std::vector<operation_ptr> _ops;

    protected:
        /* Creates a Flo store given a set of node elements and a
         * list of operations to be performed. */
        flo(std::map<std::string, node_ptr>& nodes,
            std::vector<operation_ptr>& ops)
            : _nodes(nodes),
              _node_vec(),
              _ops(ops)
            {
                for (const auto &pair : _nodes)
                    _node_vec.push_back(pair.second);
            }

    public:
        /* Iterates through this program, either by nodes or by
         * operations. */
        const std::vector<operation_ptr>& operations(void) const
            { return _ops; }
        const std::vector<node_ptr>& nodes(void) const { return _node_vec; }

        /* Adds a node to the list of nodes. */
        void add_node(node_ptr n)
            {
                _nodes[n->name()] = n;
                _node_vec.push_back(n);
            }

        /* Adds an operation to the list of operations, additionally
         * adding every node if they don't already exist. */
        void add_op(operation_ptr o)
            {
                for (const auto& node: o->operands()) {
                    auto l = _nodes.find(node->name());
                    if (l == _nodes.end())
                        add_node(node);
                }

                _ops.push_back(o);
                o->try_infer_width();
            }

        /* These are just like the single-value version, but they add
         * many nodes/ops. */
        void add_nodes(const std::vector<node_ptr>& nodes)
            {
                for (auto it = nodes.begin(); it != nodes.end(); ++it)
                    add_node(*it);
            }
        void add_ops(const std::vector<operation_ptr>& ops)
            {
                for (auto it = ops.begin(); it != ops.end(); ++it)
                    add_op(*it);
            }

        /* Re-canonicalizes an already existing Flo file.  Essentially
         * this means re-inferring widths and such. */
        void canon(void)
            {
                /* Infer the widths of everything. */
                size_t old_remaining_unknowns = 0;
                for (size_t i = 0; i < nodes().size(); ++i) {
                    for (const auto& op: this->operations())
                        op->try_infer_width();

                    /* Check to see how many unknown widths still
                     * remain -- if there's none then we're done with
                     * this phase and can continue, otherwise attempt
                     * to keep going. */
                    size_t remaining_unknowns = 0;
                    for (const auto& op: this->operations())
                        for (const auto& node: op->operands())
                            if (node->known_width() == false)
                                remaining_unknowns++;

                    if (remaining_unknowns == 0)
                        break;

                    if (remaining_unknowns == old_remaining_unknowns)
                        break;
                    old_remaining_unknowns = remaining_unknowns;
                }

                /* If we've made it this far then hopefully we know
                 * the width of every operation -- if we don't then
                 * bail out. */
                {
                    size_t remaining_unknows = 0;
                    for (const auto& op: this->operations()) {
                        for (const auto& node: op->operands()) {
                            if (!node->known_width()) {
                                fprintf(stderr, "Unable to infer '%s' in ",
                                        node->name().c_str());
                                op->writeln(stderr);
                                remaining_unknows++;
                            }
                        }
                    }

                    if (remaining_unknows > 0) {
                        fprintf(stderr, "Aborting with "
                                SIZET_FORMAT " widths remaining\n",
                                remaining_unknows);
                        abort();
                    }
                }
            }

    public:
        /* Parses the given file as a Flo file. */
        static const std::shared_ptr<flo>
        parse(const std::string filename,
              node_create_func_t node_func)
            {
                return parse_help<flo>(filename, node_func);
            }

        static const std::shared_ptr<flo>
        parse(const std::string filename)
            {
                return parse_help<flo>(filename, create_node);
            }

        /* Creates a new, empty, Flo structure that can be filled in
         * later.  Note that I didn't make this a constructor because
         * I didn't want to have an implicit empty constructor as this
         * really shouldn't being used all that much... */
        static std::shared_ptr<flo> empty(void)
            {
                std::map<std::string, node_ptr> nodes;
                std::vector<operation_ptr> ops;
                return std::shared_ptr<flo>(new flo(nodes, ops));
            }

        /* A default function for creating a new node, which can be
         * overridder by the user in case they need more information
         * to construct their class. */
        static node_ptr create_node(const std::string name,
                                    const unknown<size_t>& width,
                                    const unknown<size_t>& depth,
                                    bool is_mem,
                                    bool is_const,
                                    unknown<size_t> dfdepth,
                                    const unknown<std::string>& posn)
            {
                return std::make_shared<node_t>(name,
                                                width,
                                                depth,
                                                is_mem,
                                                is_const,
                                                dfdepth,
                                                posn);
            }

    protected:
        /* Here's the code that does the actual parsing work -- note
         * that this should only be used by subclasses of Flo, the
         * above non-templated version is the correct one to use
         * otherwise.  This version allows you to specify what sort of
         * flo you need to output in case you need to */
        template <class flo_t>
        static const std::shared_ptr<flo_t>
        parse_help(const std::string filename,
                   node_create_func_t node_func)
            {
                /* Reads a file line by line, saving it into memory. */
                std::vector<std::shared_ptr<filenode>> lines;
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
                std::map<std::string, node_ptr> lnodes;
                for (auto it = lines.begin(); it != lines.end(); ++it) {
                    auto line = *it;
                    auto node = node::parse<node_t>(line->d,
                                                    line->opcode,
                                                    line->width,
                                                    line->s,
                                                    node_func
                        );
                    if (node != NULL) {
                        lnodes[line->d] = node;
                        lnodes[node->name()] = node;
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
                    auto op = operation_t::template parse<operation_t>(
                        lnodes,
                        line->d,
                        line->opcode,
                        line->width,
                        line->s,
                        node_func
                        );
                    if (op != NULL)
                        ops.push_back(op);
                }

                /* At this point we've got the operation list, but we
                 * still need to perform width inference. */
                size_t old_remaining_unknowns = 0;
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
                    for (auto it = ops.begin(); it != ops.end(); ++it) {
                        auto op = *it;

                        for (const auto& node: op->operands())
                            if (node->known_width() == false)
                                remaining_unknowns++;
                    }
                    if (remaining_unknowns == 0)
                        break;

                    if (remaining_unknowns == old_remaining_unknowns)
                        break;
                    old_remaining_unknowns = remaining_unknowns;
                }

                /* If we've made it this far then hopefully we know
                 * the width of every operation -- if we don't then
                 * bail out. */
                {
                    size_t remaining_unknows = 0;
                    for (auto it = ops.begin(); it != ops.end(); ++it) {
                        auto op = *it;

                        for (const auto& node: op->operands()) {
                            if (!node->known_width()) {
                                fprintf(stderr, "Unable to infer '%s' in ",
                                        node->name().c_str());
                                op->writeln(stderr);
                                remaining_unknows++;
                            }
                        }
                    }

                    if (remaining_unknows > 0) {
                        fprintf(stderr, "Aborting with "
                                SIZET_FORMAT " widths remaining\n",
                                remaining_unknows);
                        abort();
                    }
                }

                /* Make an attempt to schedule this computation in
                 * dataflow order to something can actually be
                 * computed using this program. */
                old_remaining_unknowns = 0;
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
                        if (!op->known_dfdepth())
                            remaining_unknowns++;
                    }
                    if (remaining_unknowns == 0)
                        break;

                    if (remaining_unknowns == old_remaining_unknowns)
                        break;
                    old_remaining_unknowns = remaining_unknowns;
                }

                /* If we've made it this far then hopefully we've
                 * scheduled every operation -- if we don't then bail
                 * out. */
                {
                    size_t remaining_unknows = 0;
                    for (auto it = ops.begin(); it != ops.end(); ++it) {
                        auto op = *it;
                        if (!op->known_dfdepth()) {
                            fprintf(stderr, "Unable to schedule node '%s'\n",
                                    op->d()->name().c_str());
                            remaining_unknows++;
                        }
                    }

                    if (remaining_unknows > 0) {
                        for (auto it = ops.begin(); it != ops.end(); ++it) {
                            auto op = *it;
                            op->writeln_debug(stderr);
                        }

                        fprintf(stderr, "Aborting with "
                                SIZET_FORMAT " nodes remaining\n",
                                remaining_unknows);
                        abort();
                    }
                }

                std::sort(ops.begin(), ops.end(), &operation_t::cmp_sched);

                return std::shared_ptr<flo_t>(new flo_t(nodes, ops));
            }
    };
}

#endif
