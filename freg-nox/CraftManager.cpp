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
#include <QFile>
#include <QByteArray>
#include <QTextStream>
#include <QStringList>
#include "CraftManager.h"

CraftManager craft_manager;

// CraftItem section
    CraftItem::CraftItem(const int _kind, const int _sub, const int _num) :
            kind(_kind),
            sub (_sub),
            num (_num)
    {}

    quint8 CraftItem::GetKind() const { return kind; }
    quint8 CraftItem::GetSub () const { return sub;  }
    quint8 CraftItem::GetNum () const { return num;  }

// CraftList section
    CraftList:: CraftList() : size(0) {}
    CraftList::~CraftList() {
        for (quint8 i=0; i<size; ++i) {
            delete materials[i];
        }
    }
    CraftList & CraftList::operator<<(const CraftItem * const new_item) {
        if ( size < INV_SIZE-2 ) {
            materials[size++] = new_item;
        }
        return *this;
    }
// CraftManager section
CraftManager::CraftManager() {
    QDir dir("recipes");
    QStringList recipesNames = dir.entryList();
    if ( recipesNames.isEmpty() ) {
        recipesList = 0;
        recipesSubsList = 0;
        return;
    } // else:
    for (auto & recipeName : recipesNames) {
        fprintf(stderr, "found recipes: %s\n", qPrintable(recipeName));
    }
    /*
    QFile file("texts/recipes.txt");
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        fputs("No recipes file found.\n", stderr);
        return;
    }
    while ( !file.atEnd() ) {
        const QByteArray rec_arr = file.readLine();
        if ( rec_arr.isEmpty() ) {
            fputs("Recipes read error.\n", stderr);
            break;
        }
        QTextStream in(rec_arr, QIODevice::ReadOnly | QIODevice::Text);
        CraftRecipe * const recipe = new CraftRecipe;
        for (;;) {
            CraftItem * const item = new CraftItem;
            item->num = 0;
            in >> item->num >> item->kind >> item->sub;
            if ( item->num ) {
                recipe->append(item);
            } else {
                delete item;
                break;
            }
        }
        recipes.append(recipe);
    }
*/}

CraftManager::~CraftManager() {/*
    for (ushort j=0; j<recipes.size(); ++j) {
        for (ushort i=0; i<recipes.at(j)->size(); ++i) {
            delete recipes.at(j)->at(i);
        }
        delete recipes.at(j);
    }
*/}

bool CraftManager::MiniCraft(CraftItem * /*item*/, CraftItem * /*result*/) const {/*
    CraftRecipe recipe;
    recipe.append(&item);
    return Craft(recipe, result);*/
    return false;
}

bool CraftManager::Craft(CraftList * /*recipe*/, CraftList * /*result*/, const int /*sub*/) const {/*
    const ushort size = recipe.size();
    for (ushort i=0; i<recipes.size(); ++i) {
        if ( recipes.at(i)->size() != size+1 ) {
            continue;
        }
        ushort j = 0;
        for ( ; j < size
                && recipes.at(i)->at(j)->num  == recipe.at(j)->num
                && recipes.at(i)->at(j)->kind == recipe.at(j)->kind
                && recipes.at(i)->at(j)->sub  == recipe.at(j)->sub; ++j);
        if ( j == size ) {
            result.num  = recipes.at(i)->at(j)->num;
            result.kind = recipes.at(i)->at(j)->kind;
            result.sub  = recipes.at(i)->at(j)->sub;
            return true;
        }
    }*/
    return false;
}
