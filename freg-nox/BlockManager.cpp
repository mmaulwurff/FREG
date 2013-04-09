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

BlockManager block_manager;

BlockManager::BlockManager() {
	for(ushort sub=0; sub<=AIR; ++sub) {
		normals[sub]=new Block(sub);
		normals[sub]->SetInMemoryChunk(true);
	}
}
BlockManager::~BlockManager() {
	for(ushort sub=0; sub<=AIR; ++sub) {
		delete normals[sub];
	}
}

Block * BlockManager::NormalBlock(const subs sub) {
	return normals[sub];
}

Block * BlockManager::NewBlock(subs sub, const kinds kind) {
	if ( sub > AIR ) {
		fprintf(stderr,
			"Don't know such substance: %d.\n",
			sub);
		sub=STONE;
	}
	switch ( kind ) {
		case BLOCK:  return new Block(sub);
		case GRASS:  return new Grass();
		case PICK:   return new Pick(sub);
		case PLATE:  return new Plate(sub);
		case ACTIVE: return new Active(sub);
		case LADDER: return new Ladder(sub);
		case WEAPON: return new Weapon(sub);
		case BUSH:   return new Bush();
		case CHEST:  return new Chest(sub);
		case PILE:   return new Pile  ();
		case DWARF:  return new Dwarf ();
		case RABBIT: return new Rabbit();
		case DOOR:   return new Door  (sub);
		case LIQUID: return new Liquid(sub);
		//case CLOCK:  return new Clock(GetWorld(), sub);
		case WORKBENCH: return new Workbench(sub);
		default:
			fprintf(stderr,
				"Shred::NewBlock: unlisted kind: %d\n",
				kind);
			return new Block(sub);
	}
}

Block * BlockManager::BlockFromFile(QDataStream & str) {
	quint16 kind, sub;
	bool normal;
	str >> kind >> sub >> normal;
	if ( normal ) {
		return NormalBlock(static_cast<subs>(sub));
	}

	//if some kind will not be listed here,
	//blocks of this kind just will not load,
	//unless kind is inherited from Inventory class or one
	//of its derivatives - in this case this may cause something bad.
	switch ( kind ) {
		case BLOCK:  return new Block (str, sub);
		case PICK:   return new Pick  (str, sub);
		case PLATE:  return new Plate (str, sub);
		case LADDER: return new Ladder(str, sub);
		case WEAPON: return new Weapon(str, sub);

		case BUSH:   return new Bush (str);
		case CHEST:  return new Chest(str, sub);
		case WORKBENCH: return new Workbench(str, sub);

		case RABBIT: return new Rabbit(str);
		case DWARF:  return new Dwarf (str);
		case PILE:   return new Pile  (str);
		case GRASS:  return new Grass (str);
		case ACTIVE: return new Active(str, sub);
		case LIQUID: return new Liquid(str, sub);
		case DOOR:   return new Door  (str, sub);

		//case CLOCK:  return new Clock(str, world, sub);
		default:
			fprintf(stderr,
				"Shred::BlockFromFile: unlisted kind: %d.\n",
				kind);
			return NormalBlock(static_cast<subs>(sub));
	}
}

void BlockManager::DeleteBlock(Block * const block) {
	if ( !block ) {
		return;
	}
	if ( block!=NormalBlock(static_cast<subs>(block->Sub())) )
	{
		delete block;
	}
}
