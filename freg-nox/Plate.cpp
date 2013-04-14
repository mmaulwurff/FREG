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

QString & Plate::FullName(QString & str) const {
	switch ( Sub() ) {
		case WOOD: return str="Wooden board";
		case IRON: return str="Iron plate";
		case STONE: return str="Stone slab";
		default:
			fprintf(stderr,
				"Plate::FullName: unlisted sub: %d",
				Sub());
			return str="Strange plate";
	}
}

int Plate::Kind() const { return PLATE; }

Block * Plate::DropAfterDamage() const { return block_manager.NewBlock(PLATE, Sub()); }

int Plate::BeforePush(const int) { return JUMP; }

float Plate::TrueWeight() const { return 10; }

Plate::Plate(const int sub)
		:
		Block(sub, NONSTANDARD)
{}

Plate::Plate(
		QDataStream & str,
		const int sub)
		:
		Block(str, sub, NONSTANDARD)
{}
