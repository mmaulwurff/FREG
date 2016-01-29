    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2015 Alexander 'mmaulwurff' Kromm
    *  mmaulwurff@gmail.com
    *
    * This file is part of FREG.
    *
    * FREG is free software: you can redistribute it and/or modify
    * it under the terms of the GNU General Public License as published by
    * the Free Software Foundation, either version 3 of the License, or
    * (at your option) any later version.
    *
    * FREG is distributed in the hope that it will be useful,
    * but WITHOUT ANY WARRANTY; without even the implied warranty of
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    * GNU General Public License for more details.
    *
    * You should have received a copy of the GNU General Public License
    * along with FREG. If not, see <http://www.gnu.org/licenses/>. */

#ifndef UTILITY_H
#define UTILITY_H

#include <algorithm>
#include <type_traits>

/// Checks if value is in bounds [lowerBound, upperBound).
template<typename T>
bool IsInBounds(const T value, const T lowerBound, const T upperBound) {
    return (lowerBound <= value && value < upperBound);
}

template<typename T>
constexpr T mBound(const T lower, const T n, const T upper) {
    static_assert(std::is_arithmetic<T>::value, "should be ariphmetic.");
    return std::max(std::min(n, upper), lower);
}

#endif // UTILITY_H

