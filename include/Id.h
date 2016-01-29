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

#ifndef ID_H
#define ID_H

#include "header.h"

struct KindSub {
    kinds kind;
    subs  sub;
};

struct Id {
    Id(const kinds kind, const subs sub) : kindSub{ kind, sub } {}
    explicit Id(const int setId) : id(setId) {}

    bool operator==(const Id& other) const { return id == other.id; }

    union {
        KindSub kindSub;
        uint16_t id;
    };
};

#endif // ID_H
