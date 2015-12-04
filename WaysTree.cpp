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

#include "WaysTree.h"
#include <algorithm>

#define TREE_TEST 0
#if TREE_TEST
#include <QFile>
#include <QTextStream>
#endif

const std::vector<WaysTree*>& WaysTree::GetNexts() const { return nexts; }

void WaysTree::operator+=(const WaysTree* const new_chain) {
    if ( new_chain->nexts.empty() ) {
        delete new_chain;
        return;
    }
    const unsigned next_node_index = std::find_if(nexts.cbegin(), nexts.cend(),
        [&](const WaysTree* const node) {
            return *node == *new_chain->nexts[0];
    }) - std::begin(nexts);
    if ( next_node_index < nexts.size() ) { // found
        *nexts[next_node_index] += new_chain->nexts[0];
    } else {
        nexts.push_back(new_chain->nexts[0]);
    }
    delete new_chain;
}

bool WaysTree::operator==(const WaysTree& other) const {
    return  other.X() == X() &&
            other.Y() == Y() &&
            other.Z() == Z();
}

WaysTree::WaysTree()
    : Xyz(0, 0, 0)
    , nexts()
{
    const int MAX = 4;
    for (int x=-MAX; x<=MAX; ++x)
    for (int y=-MAX; y<=MAX; ++y)
    for (int z=-MAX; z<=MAX; ++z) {
        if ( abs(x)!=MAX && abs(y)!=MAX && abs(z)!=MAX ) continue;

        WaysTree* const root = new WaysTree(0, 0, 0); // new chain
        WaysTree* tail = root;

        int i=0, j=0, k=0;
        while ( not (i==x*MAX && j==y*MAX && k==z*MAX) ) {
            i += x;
            j += y;
            k += z;
            tail->nexts.push_back(new WaysTree(i/MAX, j/MAX, k/MAX));
            tail = tail->nexts[0];
        }

        *this += root;
    }

    Print(0);
}

#if TREE_TEST
void WaysTree::Print(const int level) const {
    QFile file("ATreeOut.csv");
    file.open(QIODevice::Append | QIODevice::Text);
    QTextStream stream(&file);
    for (int l=0; l<level; ++l) {
        stream << ";";
    }
    stream << X() << ";" << Y() << ";" << Z() << endl;
    for (WaysTree* const branch : nexts) {
        branch->Print(level + 1);
    }
}
#else
void WaysTree::Print(int) const {}
#endif

WaysTree::WaysTree(const int x, const int y, const int z) :
        Xyz(x, y, z),
        nexts()
{}

void WaysTree::Clear() const {
    for (WaysTree* const branch : nexts) {
        branch->Clear();
        delete branch;
    }
}
