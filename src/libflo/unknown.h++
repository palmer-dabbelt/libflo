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

#ifndef LIBFLO__UNKNOWN_HXX
#define LIBFLO__UNKNOWN_HXX

#include <stdio.h>
#include <stdlib.h>

namespace libflo {
    /* Stores a value that may or may not be known. */
    template<class T> class unknown {
    private:
        bool _known;
        T _value;

    public:
        unknown(void)
            : _known(false),
              _value()
            {
            }

        unknown(const T& value)
            : _known(true),
              _value(value)
            {
            }

        unknown(const unknown<T>& o)
            : _known(o._known),
              _value(o._value)
            {
            }

    public:
        bool known(void) const { return _known; }
        const T& value(void) const
            {
                if (_known)
                    return _value;

                fprintf(stderr, "Read value of unknown!\n");
                abort();
            }
    };
}

#endif
