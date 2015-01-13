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

#ifndef CRAFTMANAGER_H
#define CRAFTMANAGER_H

#include "header.h"

struct CraftItem final {
    CraftItem & operator=(const CraftItem &) = delete;

    bool operator<(const CraftItem & item) const;

    const int num;
    const int kind;
    const int sub;
}; // CraftItem

/** \class CraftList CraftManager.h
    * \brief This class represents craft recipe.
    *
    * It stores recipe like this:
    * (QList::size()-products_number) materials (sorted)
    * (products_number) products.
    * Comparison (==) of CraftLists is done by materials. */
class CraftList final {
public:
    explicit CraftList(int materials_number);
    ~CraftList();

    CraftList & operator=(const CraftList &) = delete;

    void Sort();
    void LoadItems(const QJsonArray &);
    int  size() const { return items.size(); }
    void clear();
    int  GetMaterialsNumber() const { return materialsNumber; }
    CraftItem * at(const int i) const { return items.at(i); }

    void operator<<(CraftItem *);
    bool operator==(const CraftList &) const;

private:
    const int materialsNumber;
    QList<CraftItem *> items;
}; // CraftList

class CraftManager final {
public:
     CraftManager();
    ~CraftManager();

    static bool MiniCraft(CraftItem **);
    static bool Craft(CraftList * items, int sub);

private:
    Q_DISABLE_COPY(CraftManager)

    bool CraftSub(CraftList * items, int sub) const;

    QList<CraftList *> recipesList[SUB_COUNT];

    static CraftManager * craftManager;
}; // CraftManager

#endif // CRAFTMANAGER_H
