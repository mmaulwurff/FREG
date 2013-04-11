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

#ifndef BLOCKMANAGER_H
#define BLOCKMANAGER_H

#include <QDataStream>
#include "header.h"

class Block;

class BlockManager {
	public:
	BlockManager();
	~BlockManager();

	Block * NormalBlock(int sub);
	Block * NewBlock(int kind, int sub=STONE);
	Block * BlockFromFile(QDataStream &);
	void DeleteBlock(Block * block);

	private:
	Block * normals[AIR+1];

	int memory_pos;
	int memory_size;
	void * memory_chunk;
	template <typename Thing>
	Thing * New(const int sub);
}; //class BlockManager

extern BlockManager block_manager;

#endif
