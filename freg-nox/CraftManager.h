	/*
	*This file is part of FREG.
	*
	*FREG is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*FREG is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#ifndef CRAFTMANAGER_H
#define CRAFTMANAGER_H

#include <QFile>
#include <QByteArray>
#include <QTextStream>

typedef struct {
	ushort num;
	int kind;
	int sub;
} craft_item;
typedef QList<craft_item *> craft_recipe;

class CraftManager {
	public:

	bool MiniCraft(craft_item & item, craft_item & result) {
		craft_recipe recipe;
		recipe.append(&item);
		return Craft(recipe, result);
	}
	bool Craft(const craft_recipe & recipe, craft_item & result);
	
	CraftManager() {
		QFile file("recipes.txt");
		if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			fputs("No recipes file found.\n", stderr);
			return;
		}
		while ( !file.atEnd() ) {
			QByteArray rec_arr=file.readLine();
			if ( rec_arr.isEmpty() ) {
				qDebug("recipes read error.");
				break;
			}
			QTextStream in(rec_arr, QIODevice::ReadOnly | QIODevice::Text);
			craft_recipe * recipe=new craft_recipe;
			for (;;) {
				craft_item * item=new craft_item;
				item->num=0;
				in >> item->num >> item->kind >> item->sub;
				if ( !item->num ) {
					delete item;
					break;
				}
				else
					recipe->append(item);
			}
			recipes.append(recipe);
		}
	}
	~CraftManager() {
		for (ushort j=0; j<recipes.size(); ++j) {
			for (ushort i=0; i<recipes.at(j)->size(); ++i)
				delete recipes.at(j)->at(i);
			delete recipes.at(j);
		}
	}

	private:
	QList<craft_recipe *> recipes;
}; //class CraftManager

extern CraftManager craft_manager;

#endif
