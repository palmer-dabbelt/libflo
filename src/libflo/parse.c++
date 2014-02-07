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

#include "parse.h++"
#include <stdio.h>
using namespace libflo;

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

const node_list libflo::parse(const std::string &filename)
{
    node_list out;

    FILE *f = fopen(filename.c_str(), "r");
    if (f == NULL) {
        fprintf(stderr, "Unable to open flo file '%s'\n", filename.c_str());
        abort();
    }

    char buffer[LINE_MAX];
    while (fgets(buffer, LINE_MAX, f) != NULL)
        out.add(node::parse(buffer));

    return out;
}
