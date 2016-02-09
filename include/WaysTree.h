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

#include "Xyz.h"
#include <vector>

class WaysTree : public Xyz {
public:
    WaysTree();

    const std::vector<WaysTree*>& GetNexts() const;

    void Clear() const;

private:
    WaysTree(int x, int y, int z);

    void operator+=(const WaysTree* new_chain);
    bool operator==(const WaysTree&) const;

    void Print(int level) const;

    std::vector<WaysTree*> nexts;
};

#endif // WAYS_TREE_H