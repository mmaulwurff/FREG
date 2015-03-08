#ifndef WAYSTREE_H
#define WAYSTREE_H

#include <vector>
#include "Xyz.h"

class WaysTree : public Xyz {
public:
    WaysTree();

    const std::vector<WaysTree*>& GetNexts() const;

    void Clear() const;

private:
    WaysTree(int x, int y, int z);

    void operator+=(WaysTree* new_chain);
    bool operator==(const WaysTree&) const;

    void Print(int level) const;

    std::vector<WaysTree*> nexts;
};

#endif // WAYSTREE_H
