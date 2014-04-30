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

#include "globals.h++"
#include <stdlib.h>
#include <stdio.h>
using namespace libflo;

#define IS_SET(name) (getenv("LIBFLO__" name) != NULL)

static void init(void) __attribute__((constructor));

bool globals::allow_bad_widths = false;

void init(void)
{
    globals::allow_bad_widths = IS_SET("ALLOW_BAD_WIDTHS");
    if (globals::allow_bad_widths == true) {
        fprintf(stderr, "WARNING: strict width checking disabled!\n");
    }
}
