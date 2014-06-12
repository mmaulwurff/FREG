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

class QTextStream;

struct CraftItem final {
    CraftItem() = delete;
    CraftItem(int num, int id);

    CraftItem & operator=(const CraftItem &) = delete;

    bool operator< (const CraftItem & item) const;
    bool operator!=(const CraftItem & item) const;

    const int num;
    const int id;
}; // CraftItem

/** \class CraftList CraftManager.h
    * \brief This class represent craft recipe.
    *
    * It stores recipe like this:
    * (QList::size()-products_number) materials (sorted)
    * (products_number) products.
    * Comparison (==) of CraftLists is done by materials. */
class CraftList final {
public:
     CraftList(quint8 materials_number, quint8 products_number);
    ~CraftList();

    CraftList & operator=(const CraftList &) = delete;

    void Sort();
    bool LoadItem(QTextStream &);
    int  GetSize() const;
    int  GetProductsNumber() const;
    CraftItem * GetItem(int item_position) const;

    CraftList & operator<<(CraftItem *);
    bool        operator==(const CraftList &) const;

private:
    const quint8 productsNumber;
    QList<CraftItem *> items;
}; // CraftList

class CraftManager final {
public:
     CraftManager();
    ~CraftManager();

    CraftItem * MiniCraft(ushort num, quint16 id) const;
    CraftList * Craft(CraftList * items, int sub) const;

private:
    CraftManager(const CraftManager &) = delete;
    CraftList * CraftSub(CraftList * items, int sub) const;

    int size;
    QList<CraftList *> * recipesList;
    int * recipesSubsList; // list of substances of workbench
}; // CraftManager

#endif // CRAFTMANAGER_H
