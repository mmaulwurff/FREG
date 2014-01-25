    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2014 Alexander 'mmaulwurff' Kromm
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

#ifndef CRAFTMANAGER_H
#define CRAFTMANAGER_H

#include "Inventory.h"

class CraftItem {
public:
    CraftItem(int kind, int sub, int num);

    quint8 GetKind() const;
    quint8 GetSub() const;
    quint8 GetNum() const;
private:
    const quint8 kind;
    const quint8 sub;
    const ushort num;
};

class CraftList {
public:
     CraftList();
    ~CraftList();

    CraftList & operator<<(const CraftItem *);
private:
    quint8 size;
    const CraftItem * materials[INV_SIZE-2]; // array size: 
};

class CraftManager {
public:
     CraftManager();
    ~CraftManager();

    bool MiniCraft(CraftItem * item,  CraftItem * result) const;
    bool     Craft(CraftList * items, CraftList * result, int sub) const;
private:
    CraftList ** recipesList;
    int * recipesSubsList; // list of substances of workbench 
};

extern CraftManager craft_manager;

#endif // CRAFTMANAGER_H
