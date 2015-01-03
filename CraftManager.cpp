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

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include "BlockFactory.h"
#include "CraftManager.h"

const CraftManager * craft_manager;

// CraftItem section

bool CraftItem::operator<(const CraftItem &item) const {
    return
        BlockFactory::MakeId(item.kind, item.sub) <
        BlockFactory::MakeId(     kind,      sub);
}

// CraftList section

CraftList:: CraftList(const int materials_number) :
        materialsNumber(materials_number),
        items()
{}

CraftList::~CraftList() { clear(); }

void CraftList::operator<<(CraftItem * const new_item) {
    items.append(new_item);
}

bool CraftList::operator==(const CraftList & compared) const {
    return ( GetMaterialsNumber() == compared.GetMaterialsNumber() ) &&
        std::equal(items.constBegin(), items.constBegin()+GetMaterialsNumber(),
            compared.items.constBegin(),
            [](const CraftItem * const first, const CraftItem * const second) {
                return not memcmp(first, second, sizeof(CraftItem));
        });
}

void CraftList::LoadItems(const QJsonArray & array) {
    for (const QJsonValue & value : array) {
        const QJsonObject item = value.toObject();
        items.append( new CraftItem( {item["number"].toInt(),
                blockFactory->StringToKind(item["kind"].toString()),
                BlockFactory::StringToSub (item["sub" ].toString())} ) );
    }
}

void CraftList::Sort() {
    std::sort(items.begin(), items.end(),
        [](const CraftItem * const item1, const CraftItem * const item2) {
            return *item1 < *item2;
    });
}

void CraftList::clear() {
    qDeleteAll(items);
    items.clear();
}

// CraftManager section

CraftManager::CraftManager() : recipesList() {
    for (int sub=0; sub<SUB_COUNT; ++sub) {
        QFile file(QString(":/recipes/%1.json").
            arg(BlockFactory::SubToString(sub)));
        if ( not file.open(QIODevice::ReadOnly | QIODevice::Text) ) continue;
        const QJsonArray recipes =
            QJsonDocument::fromJson(file.readAll()).array();
        for (int i=0; i<recipes.size(); ++i) {
            const QJsonObject recipeObject = recipes.at(i).toObject();
            const QJsonArray materials = recipeObject["materials"].toArray();
            CraftList * const recipe = new CraftList(materials.size());
            recipe->LoadItems(materials);
            recipe->Sort();
            recipe->LoadItems(recipeObject["products"].toArray());
            recipesList[sub].append(recipe);
        }
    }
}

CraftManager::~CraftManager() {
    for (int sub=0; sub<SUB_COUNT; ++sub) {
        qDeleteAll(recipesList[sub]);
    }
}

bool CraftManager::MiniCraft(CraftItem ** item) const {
    CraftList recipe(1);
    recipe << *item;
    if ( CraftSub(&recipe, DIFFERENT) ) {
        *item = new CraftItem(
            {recipe.at(0)->num, recipe.at(0)->kind, recipe.at(0)->sub} );
        return true;
    } else {
        return false;
    }
}

bool CraftManager::Craft(CraftList * const recipe, const int sub) const {
    return ( sub==DIFFERENT || not CraftSub(recipe, sub) ) ?
        CraftSub(recipe, DIFFERENT) : true;
}

bool CraftManager::CraftSub(CraftList * const recipe, const int sub) const {
    recipe->Sort();
    // find recipe and copy products from it
    for (const CraftList * const tried : recipesList[sub]) {
        if ( *tried == *recipe ) {
            recipe->clear();
            for (int i=tried->GetMaterialsNumber(); i<tried->size(); ++i) {
                *recipe << new CraftItem({tried->at(i)->num,
                        tried->at(i)->kind, tried->at(i)->sub});
            }
            return true;
        }
    }
    return false; // suitable recipe not found
}
