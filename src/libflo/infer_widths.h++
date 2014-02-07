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

#include "node_list.h++"

namespace libflo {
    /* Performs width inference on a list of nodes.  Chisel's Flo
     * backend doesn't attach a width to every node, but we really
     * need every node to have a width before anything interesting can
     * be done with it.  This outputs a node list where every width is
     * known, or fails trying to produce one. */
    const node_list infer_widths(const node_list &some_unknown_widths);
}
