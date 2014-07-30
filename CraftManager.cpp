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

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include "BlockManager.h"
#include "CraftManager.h"

CraftManager craft_manager;

// CraftItem section
CraftItem::CraftItem(const int num_, const int id_) :
        num(num_),
        id(id_)
{}

bool CraftItem::operator<(const CraftItem &item) const { return item.id < id; }

bool CraftItem::operator!=(const CraftItem & item) const {
    return num!=item.num || id!=item.id;
}

// CraftList section
CraftList:: CraftList(const int materials_number, const int  products_number) :
        productsNumber(products_number),
        items()
{
    items.reserve(materials_number + productsNumber);
}

CraftList::~CraftList() {
    for (auto & item : items) {
        delete item;
    }
}

CraftList & CraftList::operator<<(CraftItem * const new_item) {
    items.append(new_item);
    return *this;
}

bool CraftList::operator==(const CraftList & compared) const {
    const int materials_number=compared.GetSize()-compared.GetProductsNumber();
    if ( GetSize()-GetProductsNumber() != materials_number ) return false;
    for (int i=0; i<materials_number; ++i) {
        if ( *items.at(i) != *compared.items.at(i) ) return false;
    }
    return true;
}

bool ItemsLess(const CraftItem * item1, const CraftItem * item2) {
    return *item1 < *item2;
}

void CraftList::LoadItems(const QJsonArray & array) {
    for (int n=0; n<array.size(); ++n) {
        const QJsonObject item = array.at(n).toObject();
        items.append( new CraftItem(item["number"].toInt(),
            BlockManager::MakeId(
                BlockManager::StringToKind(item["kind"].toString()),
                BlockManager::StringToSub (item["sub" ].toString()) )) );
    }
}

void CraftList::Sort() { qSort(items.begin(), items.end(), ItemsLess); }
int  CraftList::GetSize() const { return items.size(); }
int  CraftList::GetProductsNumber() const { return productsNumber; }
CraftItem * CraftList::GetItem(const int i) const { return items.at(i); }

// CraftManager section
CraftManager::CraftManager() : recipesList() {
    QDir::current().mkdir("recipes");
    for (int sub=0; sub<LAST_SUB; ++sub) {
        QFile file(QString("recipes/%1.json").
            arg(BlockManager::SubToString(sub)));
        if ( not file.open(QIODevice::ReadOnly | QIODevice::Text) ) continue;
        const QJsonArray recipes =
            QJsonDocument::fromJson(file.readAll()).array();
        for (int i=0; i<recipes.size(); ++i) {
            const QJsonObject recipeObject = recipes.at(i).toObject();
            const QJsonArray materials = recipeObject["materials"].toArray();
            const QJsonArray products  = recipeObject["products" ].toArray();
            CraftList * const recipe =
                new CraftList(materials.size(), products.size());
            recipe->LoadItems(materials);
            recipe->Sort();
            recipe->LoadItems(products);
            recipesList[sub].append(recipe);
        }
    }
}

CraftManager::~CraftManager() {
    for (int i=0; i<LAST_SUB; ++i) {
        for (int j=0; j<recipesList[i].size(); ++j) {
            delete recipesList[i].at(j);
        }
    }
}

CraftItem * CraftManager::MiniCraft(const int num, const int id) const {
    CraftList recipe(1, 0);
    recipe << new CraftItem(num, id);
    const CraftList * const result = Craft(&recipe, DIFFERENT);
    if ( result != nullptr ) {
        CraftItem * const ret = new CraftItem(*result->GetItem(0));
        delete result;
        return ret;
    } else {
        return nullptr;
    }
}

CraftList * CraftManager::Craft(CraftList * const recipe, const int sub)
const {
    CraftList * ret;
    return ( sub==DIFFERENT || not (ret=CraftSub(recipe, sub)) ) ?
        CraftSub(recipe, DIFFERENT) : ret;
}

CraftList * CraftManager::CraftSub(CraftList * const recipe, const int sub)
const {
    recipe->Sort();
    // find recipe and copy products from it
    for (int i=0; i<recipesList[sub].size(); ++i) {
        const CraftList & tried = *recipesList[sub].at(i);
        if ( tried == *recipe ) {
            int number = tried.GetProductsNumber();
            CraftList * const result = new CraftList(number, 0);
            for (; number; --number) {
                *result <<
                    new CraftItem(*tried.GetItem(tried.GetSize()-number));
            }
            return result;
        }
    }
    return nullptr; // suitable recipe not found
}
