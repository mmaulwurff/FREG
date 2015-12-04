#ifndef ID_H
#define ID_H

#include "header.h"

struct KindSub {
    kinds kind;
    subs  sub;
};

struct Id {
    Id(const kinds kind, const subs sub) : kindSub{ kind, sub } {}
    Id(const int setId) : id(setId) {}

    bool operator==(const Id other) const { return id == other.id; }

    union {
        KindSub kindSub;
        uint16_t id;
    };
};

#endif // ID_H
