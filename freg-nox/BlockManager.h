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

class Block;
class QDataStream;

class BlockManager {
	public:
	BlockManager();
	~BlockManager();

	void SetMemorySize(ushort numShreds);
	void FreeMemory();

	Block * NormalBlock(int sub);
	Block * NewBlock(int kind, int sub=STONE);
	Block * BlockFromFile(QDataStream &);
	void DeleteBlock(Block * block);

	private:
	Block * normals[AIR+1];
	static const size_t BYTES_PER_SHRED=500000;

	size_t memoryPos;
	size_t memorySize;
	char * memoryChunk;

	template <typename Thing>
	Thing * New(int sub);
	template <typename Thing>
	Thing * New(QDataStream & str, int sub);
}; //class BlockManager

extern BlockManager block_manager;

#endif
