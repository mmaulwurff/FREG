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

#ifndef SHRED_H
#define SHRED_H

#include "world.h"
#include <QList>
#include <QString>
#include <blocks.h>

class Shred {
	Block * blocks[shred_width][shred_width][height];
	short lightMap[shred_width][shred_width][height-1];
	QString worldName;
	World * world;
	const long longitude;
	const long latitude;
	const unsigned short shredX;
	const unsigned short shredY;

	public:
	QList<Active *> activeList;
	void AddActive(Active * const);
	void RemActive(Active * const);

	World * GetWorld() const { return world; }

	void ReloadToNorth();
	void ReloadToEast();
	void ReloadToSouth();
	void ReloadToWest();

	Block * GetBlock(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;
	void SetBlock(Block *,
			const unsigned short,
			const unsigned short,
			const unsigned short);

	short LightMap(const unsigned short,
	               const unsigned short,
	               const unsigned short) const;
	void SetLightMap(const short,
			const unsigned short,
			const unsigned short,
			const unsigned short);
	void PlusLightMap(const short,
			const unsigned short,
			const unsigned short,
			const unsigned short);

	Shred(World * const,
			QString,
			const unsigned short,
			const unsigned short,
			const long,
			const long);
	~Shred();

	public:
	subs Sub(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;
	kinds Kind(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;

	int Transparent(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;

	private:
	char * FileName(char * const) const;
	char TypeOfShred(const unsigned long, const unsigned long) const;
	Block * NewNormal(subs sub) { return world->NewNormal(sub); }
	Block * BlockFromFile(FILE * const,
			const unsigned short,
			const unsigned short,
			const unsigned short);

	void NormalUnderground(const unsigned short);
	void PlantGrass();
	void TestShred();
	void NullMountain();
	void Plain();
	void Forest(const long, const long);
	void Water( const long, const long);
	void Hill(  const long, const long);
	//block combinations section (trees, buildings, etc)
	bool Tree(const unsigned short, const unsigned short,
			const unsigned short, const unsigned short);
	friend Inventory::Inventory(Shred * const,
			char * const str,
			FILE * const in);
};

#endif
