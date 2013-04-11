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

BlockManager::BlockManager()
		:
		memory_pos(0)
{
	for(ushort sub=0; sub<=AIR; ++sub) {
		normals[sub]=new Block(sub);
		normals[sub]->SetInMemoryChunk(true);
	}
}
BlockManager::~BlockManager() {
	for(ushort sub=0; sub<=AIR; ++sub) {
		delete normals[sub];
	}
	//fprintf(stderr, "memory: %d\n", memory_pos);
}

Block * BlockManager::NormalBlock(const int sub) {
	return normals[sub];
}

Block * BlockManager::NewBlock(const int kind, int sub) {
	if ( sub > AIR ) {
		fprintf(stderr,
			"BlockManager::NewBlock: Don't know such substance: %d.\n",
			sub);
		sub=STONE;
	}
	switch ( kind ) {
		//case BLOCK:  memory_pos+=sizeof(Block); return New<Block>(0, sub);
		case BLOCK:  memory_pos+=sizeof(Block); return new Block(sub);
		case GRASS:  memory_pos+=sizeof(Grass); return new Grass();
		case PICK:   memory_pos+=sizeof(Pick); return new Pick(sub);
		case PLATE:  memory_pos+=sizeof(Plate); return new Plate(sub);
		case ACTIVE: memory_pos+=sizeof(Active); return new Active(sub);
		case LADDER: memory_pos+=sizeof(Ladder); return new Ladder(sub);
		case WEAPON: memory_pos+=sizeof(Weapon); return new Weapon(sub);
		case BUSH:   memory_pos+=sizeof(Bush); return new Bush();
		case CHEST:  memory_pos+=sizeof(Chest); return new Chest(sub);
		case PILE:   memory_pos+=sizeof(Pile); return new Pile();
		case DWARF:  memory_pos+=sizeof(Dwarf); return new Dwarf();
		case RABBIT: memory_pos+=sizeof(Rabbit); return new Rabbit();
		case DOOR:   memory_pos+=sizeof(Door); return new Door(sub);
		case LIQUID: memory_pos+=sizeof(Liquid); return new Liquid(sub);
		//case CLOCK:  return new Clock(GetWorld(), sub);
		case WORKBENCH: memory_pos+=sizeof(Workbench); return new Workbench(sub);
		default:
			fprintf(stderr,
				"BlockManager::NewBlock: unlisted kind: %d\n",
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
				"BlockManager::BlockFromFile: unlisted kind: %d.\n",
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

template <typename Thing>
Thing * BlockManager::New(const int sub) {
	if ( memory_pos+sizeof(Thing) < memory_size ) {
		memory_pos+=sizeof(Thing);
		return new(memory_chunk) Thing(sub);
	} else {
		Block * new_thing=new Thing(sub);
		new_thing->SetInMemoryChunk(true);
		return new_thing;
	}
}
