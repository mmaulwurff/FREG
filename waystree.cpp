#include "WaysTree.h"
#include <algorithm>

#define TREE_TEST 0
#if TREE_TEST
#include <QFile>
#include <QTextStream>
#endif

const std::vector<WaysTree*>& WaysTree::GetNexts() const { return nexts; }

void WaysTree::operator+=(WaysTree* const new_chain) {
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

WaysTree::WaysTree() :
        Xyz(0, 0, 0),
        nexts()
{
    const int MAX_RADIUS = 4;
    for (int x=-MAX_RADIUS; x<=MAX_RADIUS; ++x)
    for (int y=-MAX_RADIUS; y<=MAX_RADIUS; ++y)
    for (int z=-MAX_RADIUS; z<=MAX_RADIUS; ++z) {
        int max = abs(abs(x) > abs(y) ? x : y);
        if ( abs(z) > max ) {
            max = abs(z);
        }
        if (max == 3) continue;

        WaysTree* const root = new WaysTree(0, 0, 0);
        WaysTree* tail = root;

        int i=0, j=0, k=0;
        while ( not (i==x*max && j==y*max && k==z*max) ) {
            i += x;
            j += y;
            k += z;
            tail->nexts.push_back(new WaysTree(i/max, j/max, k/max));
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
