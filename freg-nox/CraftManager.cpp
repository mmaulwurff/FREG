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
#include "BlockManager.h"
#include "CraftManager.h"
#include "header.h"

CraftManager craft_manager;

// CraftItem section
bool CraftItem::operator>(const CraftItem & item) const {
    return item.id > id;
}

bool CraftItem::operator!=(const CraftItem & item) const {
    return num!=item.num || id!=item.id;
}

// CraftList section
CraftList:: CraftList(
        const quint8 materials_number,
        const quint8  products_number)
    :
        productsNumber(products_number)
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

bool CraftList::CraftList::operator!=(const CraftList & compared) const {
    if ( items.size() != compared.items.size() ) {
        return true;
    } // else:
    for (int i=0; i<items.size(); ++i) {
        if ( *items.at(i) != *compared.items.at(i) ) {
            return  true;
        }
    }
    return false;
}

void CraftList::Sort() { qSort(items); }

bool CraftList::LoadItem(QTextStream & stream) {
    ushort    number;
    stream >> number;
    if ( number==0 ) {
        return false;
    } // else:
    QString   kind_string,   sub_string;
    stream >> kind_string >> sub_string;
    const quint8 kind = BlockManager::StringToKind(kind_string);
    const quint8 sub  = BlockManager::StringToSub ( sub_string);
    if ( LAST_KIND==kind || LAST_SUB==sub ) {
        return false;
    } // else;
    items.append(new CraftItem({number, BlockManager::MakeId(kind, sub)}));
    return true;
}

// CraftManager section
CraftManager::CraftManager() : size(0) {
    const QStringList recipesNames = QDir("recipes").entryList();
    if ( recipesNames.isEmpty() ) {
        return;
    } // else:
    recipesList = new QList<CraftList *>[recipesNames.size()];
    recipesSubsList = new int[recipesNames.size()];
    for (auto & recipeName : recipesNames) { // file level
        const quint8 sub = BlockManager::StringToSub(recipeName);
        if ( sub == LAST_SUB ) {
            continue;
        } // else:
        recipesSubsList[size] = sub;
        QFile file(QString("recipes/" + recipeName));
        int line = 0;
        while ( not file.atEnd() ) { // line(recipe) level
            QTextStream in(file.readLine(),
                QIODevice::ReadOnly | QIODevice::Text);
            ++line;
            int   materials_number,   products_number;
            in >> materials_number >> products_number;
            if ( materials_number==0 || products_number==0 ) {
                continue;
            }
            CraftList * const recipe =
                new CraftList(materials_number, products_number);
            while ( materials_number-- ) {
                if ( not recipe->LoadItem(in) ) {
                    fprintf(stderr, "Recipe read error: %s::%d.\n",
                        qPrintable(recipeName), line);
                    continue;
                }
            }
            recipe->Sort();
            while ( products_number-- ) {
                if ( not recipe->LoadItem(in) ) {
                    fprintf(stderr, "Recipe read error: %s::%d.\n",
                        qPrintable(recipeName), line);
                    continue;
                }
            }
            recipesList[size].append(recipe);
        }
        ++size;
    }
}

CraftManager::~CraftManager() {
    delete [] recipesList;
    delete [] recipesSubsList;
}

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
