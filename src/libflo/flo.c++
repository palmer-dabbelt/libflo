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

#include "flo.h++"
#include "filenode.h++"
#include "operation.h++"
#include <vector>
using namespace libflo;

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

flo::flo(const std::map<std::string, node_ptr>& se,
            const std::vector<operation_ptr>& ops)
    : _nodes(se),
      _ops(ops)
{
}

const flo flo::parse(const std::string filename)
{
    /* Reads a file line by line, saving it into memory. */
    std::vector<filenode> lines;
    {
        FILE *f = fopen(filename.c_str(), "r");
        if (f == NULL) {
            fprintf(stderr, "Unable to open '%s'\n", filename.c_str());
            abort();
        }

        char buffer[LINE_MAX];
        while (fgets(buffer, LINE_MAX, f) != NULL) 
            lines.push_back(filenode::parse(buffer));
    }

    /* The first thing we need to do is produce a hash-table of all
     * the node elements -- this is used later when looking up these
     * elements.  Essentially this keeps all the string lookups inside
     * here. */
    std::map<std::string, node_ptr> nodes;
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        auto line = *it;
        auto node = node::parse(line.d, line.opcode, line.width, line.s);
        if (node != NULL) {
            nodes[node->name()] = node;
        }
    }

    /* Now we need to produce a map of operations, which consist of
     * the actual computation.  In addition to actually producing the
     * operation structures, here we handle generating nodes for every
     * constant. */
    std::vector<operation_ptr> ops;
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        auto line = *it;
        auto op = operation::parse(nodes, line.d, line.opcode,
                                   line.width, line.s);
        if (op != NULL)
            ops.push_back(op);
    }

    /* At this point we've got the operation list, but we still need
     * to perform width inference. */
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (auto it = ops.begin(); it != ops.end(); ++it) {
            auto op = *it;
            op->try_infer_width();
        }

        /* Check to see how many unknown widths still remain -- if
         * there's none then we're done with this phase and can
         * continue, otherwise attempt to keep going. */
        size_t remaining_unknowns = 0;
        for (auto it = nodes.begin(); it != nodes.end(); ++it) {
            auto node = it->second;
            if (!node->known_width())
                remaining_unknowns++;
        }
        if (remaining_unknowns == 0)
            break;
    }

    /* If we've made it this far then hopefully we know the width of
     * every operation -- if we don't then bail out. */
    {
        size_t remaining_unknows = 0;
        for (auto it = nodes.begin(); it != nodes.end(); ++it) {
            auto node = it->second;
            if (!node->known_width()) {
                fprintf(stderr, "Unable to infer width for node '%s'\n",
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

    return flo(nodes, ops);
}
