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

int main(int argc, const char **argv)
{
    if (argc <= 1 || argc > 2) {
        fprintf(stderr, "%s <flo>: infer widths for a flo file\n", argv[0]);
    }

    auto ukw = libflo::parse(argv[1]);

    for (auto it = ukw.nodes(); !it.done(); ++it)
        (*it)->writeln(stdout);
}
