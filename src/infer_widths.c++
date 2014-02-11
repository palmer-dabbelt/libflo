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

#include <libflo/parse.h++>
#include <libflo/infer_widths.h++>
#include <libflo/version.h++>
#include <string.h>
#include "version.h"

int main(int argc, const char **argv)
{
    /* Prints a simple help text if there isn't a sane set of
     * command-line arguments, which in our case means just a single
     * argument. */
    if (argc <= 1 || argc > 2) {
        fprintf(stderr, "%s <flo>: infer widths for a flo file\n", argv[0]);
    }

    /* If "--version" was passed then print out the version of this
     * program along with the version of libflo that this was linked
     * against. */
    if (strcmp(argv[1], "--version") == 0) {
        fprintf(stderr, "%s %s (using libflo %s)\n",
                argv[0], PCONFIGURE_VERSION, libflo::version());
        exit(0);
    }

    /* Opens the file specified on the command-line as a Flo file,
     * returning a list of Flo nodes. */
    auto ukw = libflo::parse(argv[1]);

    /* The list returned above may not have a width associated with
     * every operation.  Calling infer_widths() produces a new list of
     * nodes that has an explicit width attached to every
     * operation. */
    auto kw = libflo::infer_widths(ukw);

    /* Walks through the node list and writes every node to stdout.
     * This produces a properly formatted list of nodes, each of which
     * has an attached width. */
    for (auto it = kw.nodes(); !it.done(); ++it)
        (*it)->writeln(stdout);
}
