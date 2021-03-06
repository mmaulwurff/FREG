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

#ifndef WAYS_TREE_H
#define WAYS_TREE_H

#include <QtGlobal>

class WaysTree {
public:

    explicit constexpr WaysTree(const qint16 _start)
        : start(_start)
    {}

    int X() const;
    int Y() const;
    int Z() const;

    const qint16* begin() const;
    const qint16* end() const;

    bool notCenter() const;

    enum { CENTER = 0 };

private:

    int getBranchCount() const;

    const qint16 start;

    static const qint16 data[];
};

#endif // WAYS_TREE_H
