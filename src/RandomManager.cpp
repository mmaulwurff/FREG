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

#include "RandomManager.h"

RandomManager::RandomManager(const int seed)
    : Singleton<RandomManager>(this)
    , randomEngine(seed)
    , engine1Bit(seed)
    , engine2Bit(seed)
    , engine4Bit(seed)
    , engine8Bit(seed)
{}

int RandomManager::rand() { return GetInstance()->randomEngine(); }

int RandomManager::getRandBit1() { return GetInstance()->engine1Bit(); }
int RandomManager::getRandBit2() { return GetInstance()->engine2Bit(); }
int RandomManager::getRandBit4() { return GetInstance()->engine4Bit(); }
int RandomManager::getRandBit8() { return GetInstance()->engine8Bit(); }

