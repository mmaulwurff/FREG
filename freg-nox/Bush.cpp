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

#include "blocks.h"
#include "BlockManager.h"


QString & Bush::FullName(QString & str) const { return str="Bush"; }

int Bush::Kind() const { return BUSH; }

int Bush::Sub() const { return Block::Sub(); }

usage_types Bush::Use() { return Inventory::Use(); }

Inventory * Bush::HasInventory() { return Inventory::HasInventory(); }

int Bush::Movable() const { return NOT_MOVABLE; }

float Bush::TrueWeight() const { return InvWeightAll()+20; }

void Bush::Act() {
	if ( 0==rand()%(SECONDS_IN_HOUR*4) )
		Get(block_manager.NormalBlock(HAZELNUT));
}

Block * Bush::DropAfterDamage() const {
	return block_manager.NormalBlock(WOOD);
}

void Bush::SaveAttributes(QDataStream & out) const {
	Active::SaveAttributes(out);
	Inventory::SaveAttributes(out);
}

Bush::Bush(const int sub) :
		Active(sub),
		Inventory(bush_size)
{}

Bush::Bush(QDataStream & str, const int sub) :
		Active(str, sub),
		Inventory(str, bush_size)
{}

Bush::~Bush() {}
