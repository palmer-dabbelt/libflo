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

#ifndef LIBFLO__PARSE_HXX
#define LIBFLO__PARSE_HXX

#include "node_list.h++"
#include <string>

namespace libflo {
    /* This parses a file to produce a list of Flo nodes.  Note that
     * this produces a list of Flo nodes that may or may not be of
     * well-known width, you should use width inference on this if you
     * want to know all the widths. */
    const node_list parse(const std::string &filename);
}

#endif
