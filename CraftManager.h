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

#ifndef CRAFT_MANAGER_H
#define CRAFT_MANAGER_H

#include "header.h"
#include <QList>

struct CraftItem final {
    M_DISABLE_COPY(CraftItem)

    bool operator<(const CraftItem& item) const;

    const int   number;
    const kinds kind;
    const subs  sub;
}; // CraftItem

/** * This class represents craft recipe.
    *
    * It stores recipe like this:
    * (QList::size()-products_number) materials (sorted)
    * (products_number) products.
    * Comparison (==) of CraftLists is done by materials. */
class CraftList final {
public:
    explicit CraftList(int materials_number);
    ~CraftList();
    M_DISABLE_COPY(CraftList)

    int size() const;
    int GetMaterialsNumber() const;
    void Sort();
    void LoadItems(const class QJsonArray&);
    void clear();
    const CraftItem* at(int i) const;

    void operator<<(CraftItem*);
    bool operator==(const CraftList&) const;

private:
    const int materialsNumber;
    QList<CraftItem*> items;
}; // CraftList

class CraftManager final {
public:
     CraftManager();
    ~CraftManager();
    M_DISABLE_COPY(CraftManager)

    static bool MiniCraft(CraftItem**);
    static bool Craft(CraftList* items, int sub);

private:
    bool CraftSub(CraftList* items, int sub) const;

    QList<CraftList*> recipesList[SUB_COUNT];
    static CraftManager* craftManager;
}; // CraftManager

#endif // CRAFT_MANAGER_H
