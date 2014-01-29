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
#include <QTextStream>
#include "BlockManager.h"
#include "CraftManager.h"

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

bool CraftList::CraftList::operator==(const CraftList & compared) const {
    const int materials_number=compared.GetSize()-compared.GetProductsNumber();
    if ( GetSize()-GetProductsNumber() != materials_number ) {
        return false;
    } // else:
    for (int i=0; i<materials_number; ++i) {
        if ( *items.at(i) != *compared.items.at(i) ) {
            return  false;
        }
    }
    return true;
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

int CraftList::GetSize() const { return items.size(); }
int CraftList::GetProductsNumber() const { return productsNumber; }
CraftItem * CraftList::GetItem(const int i) const { return items.at(i); }

// CraftManager section
CraftManager::CraftManager() : size(0) {
    QDir::current().mkdir("recipes");
    const QStringList recipesNames = QDir("recipes").entryList();
    if ( recipesNames.isEmpty() ) {
        recipesList = 0;
        recipesSubsList = 0;
        return;
    } // else:
    // array sizes are a bit more than needed, this is ok
    recipesList = new QList<CraftList *>[recipesNames.size()];
    recipesSubsList = new int[recipesNames.size()];
    for (auto & recipeName : recipesNames) { // file level
        const quint8 sub = BlockManager::StringToSub(recipeName);
        if ( sub == LAST_SUB ) {
            continue;
        } // else:
        recipesSubsList[size] = sub;
        QFile file(QString("recipes/" + recipeName));
        file.open(QIODevice::ReadOnly | QIODevice::Text);
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
            bool ok = true;
            while ( materials_number-- ) {
                if ( not recipe->LoadItem(in) ) {
                    fprintf(stderr, "Recipe read error: %s::%d.\n",
                        qPrintable(recipeName), line);
                    ok = false;
                    break;
                }
            }
            if ( not ok ) {
                delete recipe;
                continue;
            } // else:
            recipe->Sort();
            while ( products_number-- ) {
                if ( not recipe->LoadItem(in) ) {
                    fprintf(stderr, "Recipe read error: %s::%d.\n",
                        qPrintable(recipeName), line);
                    ok = false;
                    break;
                }
            }
            if ( not ok ) {
                delete recipe;
                continue;
            } // else:
            recipesList[size].append(recipe);
        }
        ++size;
    }
}

CraftManager::~CraftManager() {
    for (int i=0; i<size; ++i) {
        for (int j=0; j<recipesList[i].size(); ++j) {
            delete recipesList[i].at(j);
        }
    }
    delete [] recipesList;
    delete [] recipesSubsList;
}

CraftItem * CraftManager::MiniCraft(const ushort num, const quint16 id) const {
    CraftList recipe(1, 0);
    recipe << new CraftItem({num, id});
    const CraftList * const result = Craft(&recipe, DIFFERENT);
    if ( result ) {
        CraftItem * const ret = new CraftItem(*result->GetItem(0));
        delete result;
        return ret;
    } else {
        return 0;
    }
}

CraftList * CraftManager::Craft(const CraftList * const recipe, const int sub)
const {
    CraftList * ret;
    return ( sub==DIFFERENT || not (ret=CraftSub(recipe, sub)) ) ?
        CraftSub(recipe, DIFFERENT) : ret;
}

CraftList * CraftManager::CraftSub(const CraftList * const recipe,
        const int sub)
const {
    // find needed recipes list
    int point = 0;
    for ( ; recipesSubsList[point] != sub && point < size; ++point);
    if ( point == size ) { //recipes list for workbench sub not found
        return 0;
    } // else:
    // find recipe and copy products from it
    for (int i=0; i<recipesList[point].size(); ++i) {
        const CraftList & tried = *recipesList[point].at(i);
        if ( tried == *recipe ) {
            int size = tried.GetProductsNumber();
            CraftList * result = new CraftList(size, 0);
            for (; size; --size) {
                *result << new CraftItem(*tried.GetItem(tried.GetSize()-size));
            }
            return result;
        }
    }
    return 0; // suitable recipe not found
}
