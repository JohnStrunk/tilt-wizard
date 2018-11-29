/*
 * Tilt-wizard
 * Copyright (C) 2018  John D. Strunk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

 #ifndef TWERROR_H
 #define TWERROR_H

#include <comdef.h>
#include <stdexcept>

class TWError : public std::runtime_error {
public:
    HRESULT error;

    TWError(const std::string &msg, HRESULT err) :
        std::runtime_error(msg + ": " + _com_error(err).ErrorMessage()),
        error(err) { }
    TWError(const std::string &msg) :
        std::runtime_error(msg), error(E_FAIL) { }
};

 #endif // TWERROR_H
