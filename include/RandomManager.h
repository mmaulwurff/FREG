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

#ifndef RANDOM_MANAGER_H
#define RANDOM_MANAGER_H

#include "Singleton.h"
#include <random>

class RandomManager : private Singleton<RandomManager> {
public:

    explicit RandomManager(int seed);

    static int rand();

    static int getRandBit1();
    static int getRandBit2();
    static int getRandBit4();
    static int getRandBit8();

private:

    std::ranlux24 randomEngine;
    std::independent_bits_engine<std::ranlux24, 1, unsigned> engine1Bit;
    std::independent_bits_engine<std::ranlux24, 2, unsigned> engine2Bit;
    std::independent_bits_engine<std::ranlux24, 4, unsigned> engine4Bit;
    std::independent_bits_engine<std::ranlux24, 8, unsigned> engine8Bit;

};

#endif // RANDOM_MANAGER_H
